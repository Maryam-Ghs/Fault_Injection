#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>

#define DIM 10
#define POOL 30
#define INIT 6
#define ITER 8
#define EPOCH 5

/* LLM input variant 5: duplicate-heavy */

void genPool(float data[POOL][DIM], int label[POOL], const float trueW[DIM], float trueB) {
    // Define 5 repeated pattern vectors
    const float patterns[5][DIM] = {
        { 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f },
        { -0.1f, -0.1f, -0.1f, -0.1f, -0.1f, -0.1f, -0.1f, -0.1f, -0.1f, -0.1f },
        { 0.2f, -0.2f, 0.2f, -0.2f, 0.2f, -0.2f, 0.2f, -0.2f, 0.2f, -0.2f },
        { -0.2f, 0.2f, -0.2f, 0.2f, -0.2f, 0.2f, -0.2f, 0.2f, -0.2f, 0.2f },
        { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }
    };
    for (int i = 0; i < POOL; ++i) {
        const float* pat = patterns[i % 5];
        for (int j = 0; j < DIM; ++j) {
            data[i][j] = pat[j];
        }
        // compute hidden label (sign of linear function)
        float acc = trueB;
        for (int j = 0; j < DIM; ++j) {
            acc = acc + data[i][j] * trueW[j];
        }
        label[i] = (acc >= 0.0f) ? 1 : -1;
    }
}

float predict(const float w[DIM], float b, const float x[DIM]) {
    float sum = b;
    for (int k = 0; k < DIM; ++k) {
        sum = sum + x[k] * w[k];
    }
    return sum;
}

void trainModel(const float pool[POOL][DIM], const int poolLbl[POOL],
                const int idx[], int cnt,
                float w[DIM], float &b) {
    for (int e = 0; e < EPOCH; ++e) {
        for (int p = 0; p < cnt; ++p) {
            int i = idx[p];
            float out = predict(w, b, pool[i]);
            int pred = (out >= 0.0f) ? 1 : -1;
            int err = poolLbl[i] - pred;
            if (err != 0) {
                for (int d = 0; d < DIM; ++d) {
                    w[d] = w[d] + (float)err * pool[i][d];
                }
                b = b + (float)err;
            }
        }
    }
}

int selectUncertain(const float pool[POOL][DIM], const float w[DIM], float b,
                    const bool used[POOL], int cand[]) {
    float bestScore = 1e9f;
    int bestIdx = -1;
    for (int i = 0; i < POOL; ++i) {
        if (used[i]) continue;
        float val = predict(w, b, pool[i]);
        float score = std::fabs(val);
        if (score < bestScore) {
            bestScore = score;
            bestIdx = i;
        }
    }
    if (bestIdx >= 0) {
        cand[0] = bestIdx;
        return 1;
    }
    return 0;
}

float evaluate(const float pool[POOL][DIM], const int poolLbl[POOL],
               const float w[DIM], float b) {
    int correct = 0;
    for (int i = 0; i < POOL; ++i) {
        float out = predict(w, b, pool[i]);
        int pred = (out >= 0.0f) ? 1 : -1;
        if (pred == poolLbl[i]) correct = correct + 1;
    }
    return (float)correct / POOL;
}

int main() {
    std::srand((unsigned)std::time(0));

    // hidden true model with uniform weights and zero bias
    float trueW[DIM];
    for (int i = 0; i < DIM; ++i) {
        trueW[i] = 0.3f;
    }
    float trueB = 0.0f;

    // pool data
    float pool[POOL][DIM];
    int poolLbl[POOL];
    genPool(pool, poolLbl, trueW, trueB);

    // bookkeeping
    bool isLabeled[POOL] = { false };
    int labeledIdx[POOL];
    int labeledCnt = 0;

    // initial deterministic labeling (first INIT points)
    while (labeledCnt < INIT) {
        int r = labeledCnt; // deterministic selection
        if (!isLabeled[r]) {
            isLabeled[r] = true;
            labeledIdx[labeledCnt++] = r;
        }
    }

    // learner model
    float w[DIM] = { 0.0f };
    float b = 0.0f;

    // active learning loop
    for (int step = 0; step < ITER; ++step) {
        trainModel(pool, poolLbl, labeledIdx, labeledCnt, w, b);

        float acc = evaluate(pool, poolLbl, w, b);
        std::cout << "Iter " << step << " | Labeled: " << labeledCnt
                  << " | Accuracy: " << acc << std::endl;

        int cand[1];
        int got = selectUncertain(pool, w, b, isLabeled, cand);
        if (got == 0) break;

        int sel = cand[0];
        isLabeled[sel] = true;
        labeledIdx[labeledCnt++] = sel;
    }

    float finalAcc = evaluate(pool, poolLbl, w, b);
    std::cout << "Final model accuracy on full pool: " << finalAcc << std::endl;
    return 0;
}
