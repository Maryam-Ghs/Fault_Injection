#include <cstdio>
#include <cstdlib>
#include <ctime>

/* LLM input variant 1: minimal-boundary */

class AlternatingLeastSquares {
public:
    int nU, nI, k, it;
    float *rM;   // rating matrix (users x items)
    float *pF;   // user factors (users x k)
    float *qF;   // item factors (items x k)

    AlternatingLeastSquares(int users, int items, int factors, int iterations) {
        nU = users; nI = items; k = factors; it = iterations;
        rM = new float[nU * nI];
        pF = new float[nU * k];
        qF = new float[nI * k];
        for (int i = 0; i < nU * nI; ++i) rM[i] = (float)(rand() % 5);
        for (int i = 0; i < nU * k; ++i) pF[i] = (float)(rand() % 10) / 10.0f;
        for (int i = 0; i < nI * k; ++i) qF[i] = (float)(rand() % 10) / 10.0f;
    }

    ~AlternatingLeastSquares() {
        delete[] rM; delete[] pF; delete[] qF;
    }

    void onePass() {
        // ---- update user factors ----
        for (int u = 0; u < nU; ++u) {
            float *mA = new float[k * k];
            float *vecB = new float[k];
            for (int p = 0; p < k; ++p) {
                for (int q = 0; q < k; ++q) mA[p * k + q] = (p == q ? 0.1f : 0.0f);
                vecB[p] = 0.0f;
            }
            for (int i = 0; i < nI; ++i) {
                float r = rM[u * nI + i];
                float mask = (r > 0.0f) * 1.0f;                     // branch‑minimized mask
                for (int p = 0; p < k; ++p) {
                    float qp = qF[i * k + p];
                    vecB[p] += mask * qp * r;
                    for (int q = 0; q < k; ++q)
                        mA[p * k + q] += mask * qp * qF[i * k + q];
                }
            }
            // solve 2×2 system (k is fixed to 2 in this demo)
            if (k == 2) {
                float det = mA[0] * mA[3] - mA[1] * mA[2];
                float inv00 =  mA[3] / det, inv01 = -mA[1] / det;
                float inv10 = -mA[2] / det, inv11 =  mA[0] / det;
                pF[u * k + 0] = inv00 * vecB[0] + inv01 * vecB[1];
                pF[u * k + 1] = inv10 * vecB[0] + inv11 * vecB[1];
            }
            delete[] mA; delete[] vecB;
        }

        // ---- update item factors ----
        for (int i = 0; i < nI; ++i) {
            float *mA = new float[k * k];
            float *vecB = new float[k];
            for (int p = 0; p < k; ++p) {
                for (int q = 0; q < k; ++q) mA[p * k + q] = (p == q ? 0.1f : 0.0f);
                vecB[p] = 0.0f;
            }
            for (int u = 0; u < nU; ++u) {
                float r = rM[u * nI + i];
                float mask = (r > 0.0f) * 1.0f;
                for (int p = 0; p < k; ++p) {
                    float up = pF[u * k + p];
                    vecB[p] += mask * up * r;
                    for (int q = 0; q < k; ++q)
                        mA[p * k + q] += mask * up * pF[u * k + q];
                }
            }
            if (k == 2) {
                float det = mA[0] * mA[3] - mA[1] * mA[2];
                float inv00 =  mA[3] / det, inv01 = -mA[1] / det;
                float inv10 = -mA[2] / det, inv11 =  mA[0] / det;
                qF[i * k + 0] = inv00 * vecB[0] + inv01 * vecB[1];
                qF[i * k + 1] = inv10 * vecB[0] + inv11 * vecB[1];
            }
            delete[] mA; delete[] vecB;
        }
    }

    void train() {
        for (int e = 0; e < it; ++e) onePass();
    }

    void showResult() {
        for (int u = 0; u < nU; ++u) {
            for (int i = 0; i < nI; ++i) {
                float pred = 0.0f;
                for (int p = 0; p < k; ++p) pred += pF[u * k + p] * qF[i * k + p];
                printf("%6.2f ", pred);
            }
            printf("\n");
        }
    }
};

int main() {
    srand((unsigned)time(nullptr));
    AlternatingLeastSquares als(1, 1, 2, 1);   // minimal-boundary variant
    als.train();
    als.showResult();
    return 0;
}
