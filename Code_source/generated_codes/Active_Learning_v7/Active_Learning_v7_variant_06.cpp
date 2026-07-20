#include <iostream>
#include <cmath>

#define DIM 10
#define POOL 1000
#define INIT 10
#define ITER 20
#define EPOCH 5

/* LLM input variant 6: ordered-structured */

// Version #7 - Active Learning demo (loop‑heavy, stack arrays, reordered ops)

void genPool(float data[POOL][DIM], int label[POOL], const float trueW[DIM], float trueB) {
    for (int i = 0; i < POOL; ++i) {
        for (int j = 0; j < DIM; ++j) {
            // deterministic ordered values in [-1,1]
            data[i][j] = ((float)(i + j) / (POOL + DIM)) * 2.0f - 1.0f;
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
        // reordered multiplication
        sum = sum + x[k] * w[k];
    }
    return sum;
}

void trainModel(const float pool[POOL][DIM], const int poolLbl[POOL],
                const int idx[], int cnt,
                float w[DIM], float &b) {
    // simple perceptron learning
    for (int e = 0; e < EPOCH; ++e) {
        for (int p = 0; p < cnt; ++p) {
            int i = idx[p];
            float out = predict(w, b, pool[i]);
            int pred = (out >= 0.0f) ? 1 : -1;
            int err = poolLbl[i] - pred;
            if (err != 0) {
                // update rule: w = w + err * x
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
    // find point with smallest absolute margin
    float bestScore = 1e9f;
    int bestIdx = -1;
    for (int i = 0; i < POOL; ++i) {
        if (used[i]) continue;
        float val = predict(w, b, pool[i]);
        float score = std::fabs(val);
        // reorder comparison
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
        // reordered comparison
        if (pred == poolLbl[i]) correct = correct + 1;
    }
    return (float)correct / POOL;
}

int main() {
    // deterministic hidden true model
    float trueW[DIM];
    for (int i = 0; i < DIM; ++i) {
        trueW[i] = (i % 2 == 0) ? 0.5f : -0.5f;
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

    // deterministic initial labeling: first INIT points
    for (int r = 0; r < INIT; ++r) {
        isLabeled[r] = true;
        labeledIdx[labeledCnt++] = r;
    }

    // learner model
    float w[DIM] = { 0.0f };
    float b = 0.0f;

    // active learning loop
    for (int step = 0; step < ITER; ++step) {
        // train on current labeled set
        trainModel(pool, poolLbl, labeledIdx, labeledCnt, w, b);

        // evaluate on whole pool (as proxy test)
        float acc = evaluate(pool, poolLbl, w, b);
        std::cout << "Iter " << step << " | Labeled: " << labeledCnt
                  << " | Accuracy: " << acc << std::endl;

        // select most uncertain point(s)
        int cand[1];
        int got = selectUncertain(pool, w, b, isLabeled, cand);
        if (got == 0) break; // no more unlabeled

        // label the selected point (using hidden true model)
        int sel = cand[0];
        isLabeled[sel] = true;
        labeledIdx[labeledCnt++] = sel;
    }

    // final report
    float finalAcc = evaluate(pool, poolLbl, w, b);
    std::cout << "Final model accuracy on full pool: " << finalAcc << std::endl;
    return 0;
}
