#include <iostream>
#include <cmath>

/* LLM input variant 7: reverse-adversarial */

int main() {
    // Number of test pairs (adversarial, reversed order)
    int pairCount = 10;

    // Stack‑allocated vectors (3‑D) with extended size
    float vecX[10][3];
    float vecY[10][3];

    // Populate adversarial cases in reverse index order
    // 9) Large mixed extreme values
    vecX[9][0] = 9.9e7f;  vecX[9][1] = -8.8e7f; vecX[9][2] = 7.7e7f;
    vecY[9][0] = -6.6e7f; vecY[9][1] = 5.5e7f;  vecY[9][2] = -4.4e7f;

    // 8) Near‑zero vectors (tiny magnitude)
    vecX[8][0] = 1e-7f; vecX[8][1] = -1e-7f; vecX[8][2] = 2e-7f;
    vecY[8][0] = -2e-7f; vecY[8][1] = 3e-7f;  vecY[8][2] = -1e-7f;

    // 7) Anti‑parallel with large magnitude
    vecX[7][0] = -1e5f; vecX[7][1] = -2e5f; vecX[7][2] = -3e5f;
    vecY[7][0] = 1e5f;  vecY[7][1] = 2e5f;  vecY[7][2] = 3e5f;

    // 6) Parallel with small integers
    vecX[6][0] = 2; vecX[6][1] = 4; vecX[6][2] = 6;
    vecY[6][0] = 4; vecY[6][1] = 8; vecY[6][2] = 12;

    // 5) Orthogonal with mixed signs
    vecX[5][0] = 1;  vecX[5][1] = -1; vecX[5][2] = 0;
    vecY[5][0] = 1;  vecY[5][1] = 1;  vecY[5][2] = 0;

    // 4) One zero, one non‑zero (reverse order)
    vecX[4][0] = 0; vecX[4][1] = 0; vecX[4][2] = 0;
    vecY[4][0] = -7; vecY[4][1] = 8; vecY[4][2] = -9;

    // 3) Both zero vectors
    vecX[3][0] = 0; vecX[3][1] = 0; vecX[3][2] = 0;
    vecY[3][0] = 0; vecY[3][1] = 0; vecY[3][2] = 0;

    // 2) Large magnitude opposite direction but not exact anti‑parallel
    vecX[2][0] = 1e6f; vecX[2][1] = 2e6f; vecX[2][2] = -3e6f;
    vecY[2][0] = -1e6f; vecY[2][1] = -2e6f; vecY[2][2] = 3e6f;

    // 1) Near‑parallel with tiny offset
    vecX[1][0] = 1.000001f; vecX[1][1] = 2.000001f; vecX[1][2] = 3.000001f;
    vecY[1][0] = 2.000001f; vecY[1][1] = 4.000001f; vecY[1][2] = 6.000001f;

    // 0) Random moderate values
    vecX[0][0] = 13.5f; vecX[0][1] = -7.2f; vecX[0][2] = 4.8f;
    vecY[0][0] = -9.1f; vecY[0][1] = 3.3f;  vecY[0][2] = 2.2f;

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
