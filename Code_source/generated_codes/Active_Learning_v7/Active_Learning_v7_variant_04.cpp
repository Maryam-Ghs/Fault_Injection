#include <iostream>
#include <cmath>

#define DIM 10
#define POOL 1000
#define INIT 10
#define ITER 20
#define EPOCH 5

/* LLM input variant 4: signed-extremes */

void genPool(float data[POOL][DIM], int label[POOL], const float trueW[DIM], float trueB) {
    for (int i = 0; i < POOL; ++i) {
        for (int j = 0; j < DIM; ++j) {
            // deterministic values: -1, 0, 1 cycling
            int v = (i + j) % 3;          // 0,1,2
            data[i][j] = static_cast<float>(v - 1); // -1,0,1
        }
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
                    w[d] = w[d] + static_cast<float>(err) * pool[i][d];
                }
                b = b + static_cast<float>(err);
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
    return static_cast<float>(correct) / POOL;
}

int main() {
    // deterministic hidden true model with mixed signs and zero
    float trueW[DIM];
    for (int i = 0; i < DIM; ++i) {
        int mod = i % 3;
        if (mod == 0) trueW[i] = -1.0f;
        else if (mod == 1) trueW[i] = 0.0f;
        else trueW[i] = 1.0f;
    }
    float trueB = -0.5f; // mixed sign bias

    // pool data
    float pool[POOL][DIM];
    int poolLbl[POOL];
    genPool(pool, poolLbl, trueW, trueB);

    // bookkeeping
    bool isLabeled[POOL] = { false };
    int labeledIdx[POOL];
    int labeledCnt = 0;

    // deterministic initial labeling of first INIT points
    for (int i = 0; i < INIT; ++i) {
        isLabeled[i] = true;
        labeledIdx[labeledCnt++] = i;
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
