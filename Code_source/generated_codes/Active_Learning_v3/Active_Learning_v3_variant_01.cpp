/* LLM input variant 1: minimal-boundary */
#include <cstdio>
#include <cstdlib>
#include <cmath>

#define SAMPLES 3
#define FEATURES 1
#define SEED 42
#define START 1          // initial labeled pool size
#define QUERY 1           // samples queried per iteration
#define STEPS 1           // active learning cycles
#define LR 0.1f           // learning rate

/*--------------------------------------------------------------
   Helper: generate a synthetic binary classification problem.
   Features are random floats in [-1,1]. Label = 1 if sum>0 else 0.
  --------------------------------------------------------------*/
void makeData(float X[SAMPLES][FEATURES], int Y[SAMPLES])
{
    for (int i = 0; i < SAMPLES; ++i)
    {
        for (int j = 0; j < FEATURES; ++j)
        {
            X[i][j] = ((float)(rand() % 200 - 100)) / 100.0f;   // -1 … 1
        }

        // compute sum with manual unrolling
        float s = 0.0f;
        int k = 0;
        for (; k + 3 < FEATURES; k += 4)
        {
            s += X[i][k] + X[i][k + 1] + X[i][k + 2] + X[i][k + 3];
        }
        for (; k < FEATURES; ++k) s += X[i][k];

        Y[i] = (s > 0.0f) ? 1 : 0;
    }
}

/*--------------------------------------------------------------
   Helper: one perceptron epoch over a set of indices.
   Updates weights w[FEATURES] and bias b in place.
  --------------------------------------------------------------*/
void perceptronEpoch(const float X[SAMPLES][FEATURES],
                     const int   Y[SAMPLES],
                     const int   idx[],
                     int cnt,
                     float w[FEATURES],
                     float &b,
                     float  eta)
{
    for (int p = 0; p < cnt; ++p)
    {
        int i = idx[p];

        // dot product (unrolled)
        float dot = 0.0f;
        int k = 0;
        for (; k + 3 < FEATURES; k += 4)
        {
            dot += w[k] * X[i][k] + w[k + 1] * X[i][k + 1] +
                   w[k + 2] * X[i][k + 2] + w[k + 3] * X[i][k + 3];
        }
        for (; k < FEATURES; ++k) dot += w[k] * X[i][k];

        // binary prediction
        int pred = (dot + b >= 0.0f) ? 1 : 0;
        int err  = Y[i] - pred;
        if (err == 0) continue;          // already correct

        // weight update (unrolled)
        float delta = eta * (float)err;
        k = 0;
        for (; k + 3 < FEATURES; k += 4)
        {
            w[k]     += delta * X[i][k];
            w[k + 1] += delta * X[i][k + 1];
            w[k + 2] += delta * X[i][k + 2];
            w[k + 3] += delta * X[i][k + 3];
        }
        for (; k < FEATURES; ++k) w[k] += delta * X[i][k];

        b += delta;
    }
}

/*--------------------------------------------------------------
   Helper: single‑sample prediction.
  --------------------------------------------------------------*/
int predictOne(const float row[FEATURES],
               const float w[FEATURES],
               float b)
{
    float dot = 0.0f;
    int k = 0;
    for (; k + 3 < FEATURES; k += 4)
    {
        dot += w[k] * row[k] + w[k + 1] * row[k + 1] +
               w[k + 2] * row[k + 2] + w[k + 3] * row[k + 3];
    }
    for (; k < FEATURES; ++k) dot += w[k] * row[k];
    return (dot + b >= 0.0f) ? 1 : 0;
}

/*--------------------------------------------------------------
   Helper: evaluate accuracy on a given index list.
  --------------------------------------------------------------*/
void evaluate(const float X[SAMPLES][FEATURES],
              const int   Y[SAMPLES],
              const int   pool[],
              int poolSize,
              const float w[FEATURES],
              float b,
              int &accuracy)
{
    int correct = 0;
    for (int i = 0; i < poolSize; ++i)
    {
        int idx = pool[i];
        int pred = predictOne(X[idx], w, b);
        if (pred == Y[idx]) ++correct;
    }
    accuracy = poolSize ? (correct * 100) / poolSize : 0;
}

/*--------------------------------------------------------------
   Helper: pick the most uncertain samples (closest to boundary).
   Returns up to 'need' indices in sel[] (order of increasing |margin|).
  --------------------------------------------------------------*/
void pickUncertain(const float X[SAMPLES][FEATURES],
                   const int   pool[],
                   int poolSize,
                   const float w[FEATURES],
                   float b,
                   int sel[],
                   int need)
{
    // initialise selection slots
    for (int i = 0; i < need; ++i) sel[i] = -1;

    for (int p = 0; p < poolSize; ++p)
    {
        int idx = pool[p];

        // margin = |w·x + b|
        float dot = 0.0f;
        int k = 0;
        for (; k + 3 < FEATURES; k += 4)
        {
            dot += w[k] * X[idx][k] + w[k + 1] * X[idx][k + 1] +
                   w[k + 2] * X[idx][k + 2] + w[k + 3] * X[idx][k + 3];
        }
        for (; k < FEATURES; ++k) dot += w[k] * X[idx][k];
        float margin = fabsf(dot + b);

        // insert into sorted selection (ascending margin)
        for (int s = 0; s < need; ++s)
        {
            if (sel[s] == -1)
            {
                sel[s] = idx;
                break;
            }

            // compute margin of current occupant
            int curIdx = sel[s];
            float curDot = 0.0f;
            int kk = 0;
            for (; kk + 3 < FEATURES; kk += 4)
            {
                curDot += w[kk] * X[curIdx][kk] + w[kk + 1] * X[curIdx][kk + 1] +
                          w[kk + 2] * X[curIdx][kk + 2] + w[kk + 3] * X[curIdx][kk + 3];
            }
            for (; kk < FEATURES; ++kk) curDot += w[kk] * X[curIdx][kk];
            float curMargin = fabsf(curDot + b);

            if (margin < curMargin)
            {
                // shift right and insert
                for (int t = need - 1; t > s; --t) sel[t] = sel[t - 1];
                sel[s] = idx;
                break;
            }
        }
    }
}

/*--------------------------------------------------------------
   Main: runs a tiny active‑learning loop on synthetic data.
  --------------------------------------------------------------*/
int main()
{
    srand(SEED);

    // ----------------------------------------------------------
    // 1) generate data on the stack
    // ----------------------------------------------------------
    float feats[SAMPLES][FEATURES];
    int   truth[SAMPLES];
    makeData(feats, truth);

    // ----------------------------------------------------------
    // 2) initialise pools
    // ----------------------------------------------------------
    int labeled[START + STEPS * QUERY];
    for (int i = 0; i < START; ++i) labeled[i] = i;   // first START samples are labeled

    int unlabeled[SAMPLES - START];
    int ulCnt = 0;
    for (int i = START; i < SAMPLES; ++i) unlabeled[ulCnt++] = i;

    // ----------------------------------------------------------
    // 3) initialise perceptron
    // ----------------------------------------------------------
    float w[FEATURES];
    for (int i = 0; i < FEATURES; ++i) w[i] = 0.0f;
    float b = 0.0f;

    // one warm‑up epoch on the tiny seed set
    perceptronEpoch(feats, truth, labeled, START, w, b, LR);

    // ----------------------------------------------------------
    // 4) active‑learning cycles
    // ----------------------------------------------------------
    for (int step = 0; step < STEPS; ++step)
    {
        // a) select most uncertain examples
        int queried[QUERY];
        for (int i = 0; i < QUERY; ++i) queried[i] = -1;
        pickUncertain(feats, unlabeled, ulCnt, w, b, queried, QUERY);

        // b) move them to the labeled pool
        int added = 0;
        for (int i = 0; i < QUERY; ++i)
        {
            if (queried[i] == -1) break;
            labeled[START + step * QUERY + added] = queried[i];
            ++added;
        }

        // c) compact the unlabeled pool (remove chosen)
        int newUl = 0;
        for (int i = 0; i < ulCnt; ++i)
        {
            int cur = unlabeled[i];
            bool taken = false;
            for (int j = 0; j < QUERY; ++j)
                if (cur == queried[j]) { taken = true; break; }
            if (!taken) unlabeled[newUl++] = cur;
        }
        ulCnt = newUl;

        // d) retrain on the enlarged labeled set
        int totalLabeled = START + (step + 1) * QUERY;
        perceptronEpoch(feats, truth, labeled, totalLabeled, w, b, LR);

        // e) report performance on the still‑unlabeled pool
        int acc = 0;
        evaluate(feats, truth, unlabeled, ulCnt, w, b, acc);
        printf("Step %d: pool accuracy %d%%, labeled %d samples\n",
               step + 1, acc, totalLabeled);
    }

    // ----------------------------------------------------------
    // 5) final check on the whole labeled collection
    // ----------------------------------------------------------
    int finalAcc = 0;
    evaluate(feats, truth, labeled,
             START + STEPS * QUERY, w, b, finalAcc);
    printf("Final model accuracy on labeled set: %d%%\n", finalAcc);

    return 0;
}
