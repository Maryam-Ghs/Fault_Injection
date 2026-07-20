/* LLM input variant 3: zeros-and-ones */
// version #8
#include <cstdio>
#include <cmath>

int main() {
    /* ---------- data ---------- */
    int cityCount = 5;

    // Emphasize zeros and ones; using mostly zeros to create identity-like distances
    float posX[5] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
    float posY[5] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

    /* ---------- distance matrix (manual unroll) ---------- */
    float distMat[5][5];

    // row 0
    distMat[0][0] = 0.0f;
    distMat[0][1] = sqrtf((posX[0] - posX[1]) * (posX[0] - posX[1]) +
                          (posY[0] - posY[1]) * (posY[0] - posY[1]));
    distMat[0][2] = sqrtf((posX[0] - posX[2]) * (posX[0] - posX[2]) +
                          (posY[0] - posY[2]) * (posY[0] - posY[2]));
    distMat[0][3] = sqrtf((posX[0] - posX[3]) * (posX[0] - posX[3]) +
                          (posY[0] - posY[3]) * (posY[0] - posY[3]));
    distMat[0][4] = sqrtf((posX[0] - posX[4]) * (posX[0] - posX[4]) +
                          (posY[0] - posY[4]) * (posY[0] - posY[4]));

    // row 1
    distMat[1][0] = distMat[0][1];
    distMat[1][1] = 0.0f;
    distMat[1][2] = sqrtf((posX[1] - posX[2]) * (posX[1] - posX[2]) +
                          (posY[1] - posY[2]) * (posY[1] - posY[2]));
    distMat[1][3] = sqrtf((posX[1] - posX[3]) * (posX[1] - posX[3]) +
                          (posY[1] - posY[3]) * (posY[1] - posY[3]));
    distMat[1][4] = sqrtf((posX[1] - posX[4]) * (posX[1] - posX[4]) +
                          (posY[1] - posY[4]) * (posY[1] - posY[4]));

    // row 2
    distMat[2][0] = distMat[0][2];
    distMat[2][1] = distMat[1][2];
    distMat[2][2] = 0.0f;
    distMat[2][3] = sqrtf((posX[2] - posX[3]) * (posX[2] - posX[3]) +
                          (posY[2] - posY[3]) * (posY[2] - posY[3]));
    distMat[2][4] = sqrtf((posX[2] - posX[4]) * (posX[2] - posX[4]) +
                          (posY[2] - posY[4]) * (posY[2] - posY[4]));

    // row 3
    distMat[3][0] = distMat[0][3];
    distMat[3][1] = distMat[1][3];
    distMat[3][2] = distMat[2][3];
    distMat[3][3] = 0.0f;
    distMat[3][4] = sqrtf((posX[3] - posX[4]) * (posX[3] - posX[4]) +
                          (posY[3] - posY[4]) * (posY[3] - posY[4]));

    // row 4
    distMat[4][0] = distMat[0][4];
    distMat[4][1] = distMat[1][4];
    distMat[4][2] = distMat[2][4];
    distMat[4][3] = distMat[3][4];
    distMat[4][4] = 0.0f;

    /* ---------- initial tour ---------- */
    int order[5] = { 0, 1, 2, 3, 4 };

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

        // possible (i,k) pairs for N = 5:
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
