#include <iostream>
#include <cmath>

/* LLM input variant 9: medium-deterministic-random */

int main() {
    // Number of test pairs (medium‑size, diverse)
    int pairCount = 10;

    // Stack‑allocated vectors (3‑D)
    float vecX[10][3];
    float vecY[10][3];

    // Populate deterministic pseudo‑random‑looking inputs
    // 0) Both zero vectors
    vecX[0][0] = 0; vecX[0][1] = 0; vecX[0][2] = 0;
    vecY[0][0] = 0; vecY[0][1] = 0; vecY[0][2] = 0;

    // 1) Small fractional values
    vecX[1][0] = 0.1f; vecX[1][1] = -0.2f; vecX[1][2] = 0.3f;
    vecY[1][0] = -0.4f; vecY[1][1] = 0.5f; vecY[1][2] = -0.6f;

    // 2) Parallel (same direction)
    vecX[2][0] = 2; vecX[2][1] = 4; vecX[2][2] = 6;
    vecY[2][0] = 4; vecY[2][1] = 8; vecY[2][2] = 12;

    // 3) Anti‑parallel (opposite direction)
    vecX[3][0] = -3; vecX[3][1] = -6; vecX[3][2] = -9;
    vecY[3][0] = 1;  vecY[3][1] = 2;  vecY[3][2] = 3;

    // 4) Orthogonal
    vecX[4][0] = 1; vecX[4][1] = 0; vecX[4][2] = 0;
    vecY[4][0] = 0; vecY[4][1] = 1; vecY[4][2] = 0;

    // 5) Mixed integer primes
    vecX[5][0] = 7;  vecX[5][1] = -13; vecX[5][2] = 19;
    vecY[5][0] = 23; vecY[5][1] = -29; vecY[5][2] = 31;

    // 6) Large mixed values
    vecX[6][0] = 1e5f;  vecX[6][1] = -2e5f; vecX[6][2] = 3e5f;
    vecY[6][0] = -4e5f; vecY[6][1] = 5e5f;  vecY[6][2] = -6e5f;

    // 7) Small and large mix
    vecX[7][0] = 0.001f; vecX[7][1] = 1000.0f; vecX[7][2] = -0.5f;
    vecY[7][0] = 500.0f; vecY[7][1] = -0.002f; vecY[7][2] = 0.003f;

    // 8) Additional pseudo‑random set
    vecX[8][0] = 13;  vecX[8][1] = 17;  vecX[8][2] = -19;
    vecY[8][0] = -23; vecY[8][1] = 29;  vecY[8][2] = -31;

    // 9) Near‑parallel but not exact
    vecX[9][0] = 1.0f;      vecX[9][1] = 1.0f;      vecX[9][2] = 1.0f;
    vecY[9][0] = 1.000001f; vecY[9][1] = 0.999999f; vecY[9][2] = 1.0f;

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
