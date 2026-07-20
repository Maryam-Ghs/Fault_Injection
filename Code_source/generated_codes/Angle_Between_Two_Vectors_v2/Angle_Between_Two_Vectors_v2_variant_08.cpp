#include <iostream>
#include <cmath>

/* LLM input variant 8: sparse-skewed */

int main() {
    // Number of test pairs (sparse, skewed distribution)
    int pairCount = 10;

    // Stack‑allocated vectors (3‑D)
    float vecX[10][3];
    float vecY[10][3];

    // Populate sparse/skewed cases
    // 0) Both zero vectors
    vecX[0][0] = 0; vecX[0][1] = 0; vecX[0][2] = 0;
    vecY[0][0] = 0; vecY[0][1] = 0; vecY[0][2] = 0;

    // 1) X zero, Y single non‑zero component
    vecX[1][0] = 0; vecX[1][1] = 0; vecX[1][2] = 0;
    vecY[1][0] = 0; vecY[1][1] = 0; vecY[1][2] = 7;

    // 2) X single non‑zero, Y zero
    vecX[2][0] = 9; vecX[2][1] = 0; vecX[2][2] = 0;
    vecY[2][0] = 0; vecY[2][1] = 0; vecY[2][2] = 0;

    // 3) Parallel, single component
    vecX[3][0] = 4; vecX[3][1] = 0; vecX[3][2] = 0;
    vecY[3][0] = 12; vecY[3][1] = 0; vecY[3][2] = 0;

    // 4) Orthogonal, single components
    vecX[4][0] = 0; vecX[4][1] = 5; vecX[4][2] = 0;
    vecY[4][0] = 3; vecY[4][1] = 0; vecY[4][2] = 0;

    // 5) Sparse large values on middle component
    vecX[5][0] = 0; vecX[5][1] = 1e5f; vecX[5][2] = 0;
    vecY[5][0] = 0; vecY[5][1] = -2e5f; vecY[5][2] = 0;

    // 6) Mixed small values on first and third components
    vecX[6][0] = 1; vecX[6][1] = 0; vecX[6][2] = -1;
    vecY[6][0] = -1; vecY[6][1] = 0; vecY[6][2] = 1;

    // 7) Anti‑parallel, huge magnitude on first component
    vecX[7][0] = 3e4f; vecX[7][1] = 0; vecX[7][2] = 0;
    vecY[7][0] = -6e4f; vecY[7][1] = 0; vecY[7][2] = 0;

    // 8) Near‑zero angle (both have tiny same component)
    vecX[8][0] = 0; vecX[8][1] = 0; vecX[8][2] = 0.001f;
    vecY[8][0] = 0; vecY[8][1] = 0; vecY[8][2] = 0.002f;

    // 9) Single component anti‑parallel on third axis
    vecX[9][0] = 0; vecX[9][1] = 0; vecX[9][2] = 5e3f;
    vecY[9][0] = 0; vecY[9][1] = 0; vecY[9][2] = -5e3f;

    // Iterate over all pairs using while‑loops
    int idx = 0;
    while (idx < pairCount) {
        // Compute dot product (reordered summation)
        float dotProd = 0;
        int comp = 2;
        while (comp >= 0) {
            dotProd = dotProd + (vecX[idx][comp] * vecY[idx][comp]);
            comp = comp - 1;
        }

        // Compute lengths (squared) with reordered addition
        float lenSqX = 0, lenSqY = 0;
        comp = 0;
        while (comp < 3) {
            lenSqX = lenSqX + (vecX[idx][comp] * vecX[idx][comp]);
            lenSqY = lenSqY + (vecY[idx][comp] * vecY[idx][comp]);
            comp = comp + 1;
        }

        // Final lengths
        float lenX = std::sqrt(lenSqX);
        float lenY = std::sqrt(lenSqY);

        // Guard against zero‑length vectors
        if (lenX == 0.0f || lenY == 0.0f) {
            std::cout << "Pair " << idx << ": angle undefined (zero length vector)\n";
            idx = idx + 1;
            continue;
        }

        // Cosine of angle, clamped to [-1,1] to avoid NaNs from rounding
        float cosTheta = dotProd / (lenX * lenY);
        if (cosTheta > 1.0f) cosTheta = 1.0f;
        if (cosTheta < -1.0f) cosTheta = -1.0f;

        // Angle in radians then degrees (reordered multiplication)
        float rad = std::acos(cosTheta);
        float deg = rad * (180.0f / 3.1415926535f);

        std::cout << "Pair " << idx << ": angle = " << deg << " degrees\n";

        idx = idx + 1; // advance to next pair
    }

    return 0;
}
