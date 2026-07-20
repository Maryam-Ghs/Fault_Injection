/* LLM input variant 6: ordered-structured */
#include <cstdio>
#include <cmath>

int main() {
    /* ---------- data ---------- */
    int cityCount = 8;

    float posX[8] = { 0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f };
    float posY[8] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

    /* ---------- distance matrix (generated) ---------- */
    float distMat[8][8];
    for (int i = 0; i < cityCount; ++i) {
        for (int j = 0; j < cityCount; ++j) {
            if (i == j) {
                distMat[i][j] = 0.0f;
            } else {
                float dx = posX[i] - posX[j];
                float dy = posY[i] - posY[j];
                distMat[i][j] = sqrtf(dx * dx + dy * dy);
            }
        }
    }

    /* ---------- initial tour ---------- */
    int order[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };

    /* ---------- helper: tour length ---------- */
    auto tourLength = [&](int *seq) -> float {
        float sum = 0.0f;
        for (int i = 0; i < cityCount - 1; ++i) {
            sum = sum + distMat[seq[i]][seq[i + 1]];
        }
        sum = sum + distMat[seq[cityCount - 1]][seq[0]];   // return to start
        return sum;
    };

    /* ---------- 2‑Opt improvement (manual unroll) ---------- */
    bool anyChange;
    do {
        anyChange = false;

        // possible (i,k) pairs for N = 5 (kept from original):
        // (1,2), (1,3), (2,3)

        // ---- pair (1,2) ----
        {
            int a = order[0];
            int b = order[1];
            int c = order[2];
            int d = order[3];

            float oldEdges = distMat[a][b] + distMat[c][d];
            float newEdges = distMat[a][c] + distMat[b][d];
            float gain = oldEdges - newEdges;   // positive gain = improvement

            if (gain > 0.0f) {
                // reverse segment [1,2] -> just swap order[1] and order[2]
                int tmp = order[1];
                order[1] = order[2];
                order[2] = tmp;
                anyChange = true;
            }
        }

        // ---- pair (1,3) ----
        {
            int a = order[0];
            int b = order[1];
            int c = order[3];
            int d = order[4];

            float oldEdges = distMat[a][b] + distMat[c][d];
            float newEdges = distMat[a][c] + distMat[b][d];
            float gain = oldEdges - newEdges;

            if (gain > 0.0f) {
                // reverse segment [1,3]
                int tmp0 = order[1];
                int tmp1 = order[2];
                order[1] = order[3];
                order[2] = tmp1;          // middle stays
                order[3] = tmp0;
                anyChange = true;
            }
        }

        // ---- pair (2,3) ----
        {
            int a = order[1];
            int b = order[2];
            int c = order[3];
            int d = order[0];   // because edge wraps to start

            float oldEdges = distMat[a][b] + distMat[c][d];
            float newEdges = distMat[a][c] + distMat[b][d];
            float gain = oldEdges - newEdges;

            if (gain > 0.0f) {
                // reverse segment [2,3] (swap order[2] and order[3])
                int tmp = order[2];
                order[2] = order[3];
                order[3] = tmp;
                anyChange = true;
            }
        }

    } while (anyChange);

    /* ---------- output ---------- */
    float finalLen = tourLength(order);
    std::printf("Final tour length: %.3f\n", finalLen);
    std::printf("Tour order: ");
    for (int i = 0; i < cityCount; ++i) {
        std::printf("%d ", order[i]);
    }
    std::printf("\n");

    return 0;
}
