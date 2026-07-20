#include <iostream>
#include <cmath>

/* LLM input variant 5: duplicate-heavy */

int main() {
    // Number of test pairs (duplicate-heavy)
    int pairCount = 6;

    // Stack‑allocated vectors (3‑D)
    float vecX[6][3];
    float vecY[6][3];

    // Populate duplicate‑heavy test cases
    // 0) Zero vector (duplicate)
    vecX[0][0] = 0.0f; vecX[0][1] = 0.0f; vecX[0][2] = 0.0f;
    vecY[0][0] = 0.0f; vecY[0][1] = 0.0f; vecY[0][2] = 0.0f;

    // 1) Zero vector (duplicate of 0)
    vecX[1][0] = 0.0f; vecX[1][1] = 0.0f; vecX[1][2] = 0.0f;
    vecY[1][0] = 0.0f; vecY[1][1] = 0.0f; vecY[1][2] = 0.0f;

    // 2) Parallel identical vectors (all components equal)
    vecX[2][0] = 5.0f; vecX[2][1] = 5.0f; vecX[2][2] = 5.0f;
    vecY[2][0] = 5.0f; vecY[2][1] = 5.0f; vecY[2][2] = 5.0f;

    // 3) Duplicate of pair 2 but anti‑parallel
    vecX[3][0] = 5.0f; vecX[3][1] = 5.0f; vecX[3][2] = 5.0f;
    vecY[3][0] = -5.0f; vecY[3][1] = -5.0f; vecY[3][2] = -5.0f;

    // 4) Repeated pattern vectors
    vecX[4][0] = 5.0f; vecX[4][1] = -5.0f; vecX[4][2] = 5.0f;
    vecY[4][0] = -5.0f; vecY[4][1] = 5.0f; vecY[4][2] = -5.0f;

    // 5) Duplicate of pair 4
    vecX[5][0] = 5.0f; vecX[5][1] = -5.0f; vecX[5][2] = 5.0f;
    vecY[5][0] = -5.0f; vecY[5][1] = 5.0f; vecY[5][2] = -5.0f;

    // Iterate over all pairs using while‑loops
    int idx = 0;
    while (idx < pairCount) {
        // Compute dot product (reordered summation)
        float dotProd = 0.0f;
        int comp = 2;
        while (comp >= 0) {
            dotProd = dotProd + (vecX[idx][comp] * vecY[idx][comp]);
            comp = comp - 1;
        }

        // Compute lengths (squared) with reordered addition
        float lenSqX = 0.0f, lenSqY = 0.0f;
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
