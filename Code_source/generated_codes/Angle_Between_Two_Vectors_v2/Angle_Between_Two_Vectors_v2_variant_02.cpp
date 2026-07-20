/* LLM input variant 2: small-diverse */
#include <iostream>
#include <cmath>

int main() {
    // Number of test pairs (small‑diverse)
    int pairCount = 6;

    // Stack‑allocated vectors (3‑D)
    float vecX[6][3];
    float vecY[6][3];

    // Populate diverse test cases
    // 0) Both zero vectors
    vecX[0][0] = 0; vecX[0][1] = 0; vecX[0][2] = 0;
    vecY[0][0] = 0; vecY[0][1] = 0; vecY[0][2] = 0;

    // 1) One zero, one non‑zero
    vecX[1][0] = 0; vecX[1][1] = 0; vecX[1][2] = 0;
    vecY[1][0] = 3; vecY[1][1] = -4; vecY[1][2] = 5;

    // 2) Parallel (same direction, different magnitude)
    vecX[2][0] = 2; vecX[2][1] = 4; vecX[2][2] = 6;
    vecY[2][0] = 4; vecY[2][1] = 8; vecY[2][2] = 12;

    // 3) Anti‑parallel (opposite direction)
    vecX[3][0] = 1;  vecX[3][1] = -2; vecX[3][2] = 3;
    vecY[3][0] = -1; vecY[3][1] = 2;  vecY[3][2] = -3;

    // 4) Orthogonal vectors
    vecX[4][0] = 0; vecX[4][1] = 1; vecX[4][2] = 0;
    vecY[4][0] = 0; vecY[4][1] = 0; vecY[4][2] = 1;

    // 5) Large mixed values
    vecX[5][0] = 123.45f;   vecX[5][1] = -678.9f; vecX[5][2] = 0.001f;
    vecY[5][0] = -321.0f;  vecY[5][1] = 456.78f; vecY[5][2] = -0.002f;

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
