/* LLM input variant 5: duplicate-heavy */
/*  Affinity Propagation – version #5  */
#include <cstdio>
#include <cmath>

#define MAXN 5          // maximum number of data points (stack allocation)

class AffinityProp {
    int   cnt;                         // number of points
    float pt[MAXN][2];                 // coordinates
    float simMat[MAXN][MAXN];          // similarity matrix
    float respMat[MAXN][MAXN];         // responsibility matrix
    float availMat[MAXN][MAXN];        // availability matrix
    int   assign[MAXN];                // final exemplar for each point

    /*------------------------------------------------------------*/
    void makeSimilarity() {
        int i, k;
        for (i = 0; i < cnt; ++i) {
            for (k = 0; k < cnt; ++k) {
                float dx = pt[i][0] - pt[k][0];
                float dy = pt[i][1] - pt[k][1];
                float dist2 = dx * dx + dy * dy;          // temporary variable
                simMat[i][k] = -dist2;                    // negative squared distance
            }
        }
        /* preference – set to median of similarities (simple deterministic choice) */
        float pref = simMat[0][1];
        int p, q;
        for (p = 0; p < cnt; ++p) {
            for (q = p + 1; q < cnt; ++q) {
                if (simMat[p][q] > pref) pref = simMat[p][q];
            }
        }
        for (i = 0; i < cnt; ++i) simMat[i][i] = pref;
    }

    /*------------------------------------------------------------*/
    void iterate(int maxIter, float damp) {
        int it, i, k, kp, ip;
        for (it = 0; it < maxIter; ++it) {
            /* ---- update responsibilities ---- */
            for (i = 0; i < cnt; ++i) {
                for (k = 0; k < cnt; ++k) {
                    /* find max_{k' != k} (a(i,k') + s(i,k')) */
                    float best = -1e30f;
                    for (kp = 0; kp < cnt; ++kp) {
                        if (kp == k) continue;
                        float temp = availMat[i][kp] + simMat[i][kp];
                        if (temp > best) best = temp;
                    }
                    float newR = simMat[i][k] - best;          // temporary
                    respMat[i][k] = damp * respMat[i][k] + (1.0f - damp) * newR;
                }
            }

            /* ---- update availabilities ---- */
            for (k = 0; k < cnt; ++k) {                     // note changed loop order
                for (i = 0; i < cnt; ++i) {
                    if (i == k) {
                        /* self‑availability */
                        float sumPos = 0.0f;
                        for (ip = 0; ip < cnt; ++ip) {
                            if (ip == k) continue;
                            float rVal = respMat[ip][k];
                            if (rVal > 0.0f) sumPos += rVal;
                        }
                        float newA = sumPos;                 // temporary
                        availMat[i][k] = damp * availMat[i][k] + (1.0f - damp) * newA;
                    } else {
                        /* non‑self availability */
                        float sumPos = 0.0f;
                        for (ip = 0; ip < cnt; ++ip) {
                            if (ip == k || ip == i) continue;
                            float rVal = respMat[ip][k];
                            if (rVal > 0.0f) sumPos += rVal;
                        }
                        float rkk = respMat[k][k];
                        if (rkk < 0.0f) rkk = 0.0f;
                        float newA = rkk + sumPos;            // temporary
                        if (newA < 0.0f) newA = 0.0f;
                        availMat[i][k] = damp * availMat[i][k] + (1.0f - damp) * newA;
                    }
                }
            }
        }
    }

    /*------------------------------------------------------------*/
    void decideExemplars() {
        int i, k;
        for (i = 0; i < cnt; ++i) {
            float bestVal = -1e30f;
            int bestIdx = -1;
            for (k = 0; k < cnt; ++k) {
                float val = availMat[i][k] + respMat[i][k];
                if (val > bestVal) {
                    bestVal = val;
                    bestIdx = k;
                }
            }
            assign[i] = bestIdx;
        }
    }

public:
    AffinityProp(int n) : cnt(n) {
        int i, d;
        /* deterministic test vectors (2‑D points) – duplicate-heavy */
        float testPts[MAXN][2] = {
            {1.0f, 1.0f},
            {1.0f, 1.0f},
            {2.0f, 2.0f},
            {2.0f, 2.0f},
            {2.0f, 2.0f}
        };
        for (i = 0; i < cnt; ++i) {
            for (d = 0; d < 2; ++d) pt[i][d] = testPts[i][d];
        }
        /* zero matrices */
        for (i = 0; i < cnt; ++i) {
            for (d = 0; d < cnt; ++d) {
                respMat[i][d] = 0.0f;
                availMat[i][d] = 0.0f;
            }
        }
        makeSimilarity();
    }

    void run() {
        iterate(10, 0.5f);          // 10 iterations, damping factor 0.5
        decideExemplars();
    }

    void output() {
        int i;
        printf("Affinity Propagation – version #5\n");
        for (i = 0; i < cnt; ++i) {
            printf("Point %d (%.1f, %.1f) -> exemplar %d (%.1f, %.1f)\n",
                   i,
                   pt[i][0], pt[i][1],
                   assign[i],
                   pt[assign[i]][0], pt[assign[i]][1]);
        }
    }
};

/*------------------------------------------------------------*/
int main() {
    AffinityProp ap(5);   // use all 5 deterministic points (with duplicates)
    ap.run();
    ap.output();
    return 0;
}
