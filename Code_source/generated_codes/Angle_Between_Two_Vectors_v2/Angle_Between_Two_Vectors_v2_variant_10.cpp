#include <iostream>
#include <cmath>

/* LLM input variant 10: large-safe-stress */

int main() {
    // Number of test pairs – large but safe for stack memory
    const int pairCount = 1000;

    // Stack‑allocated vectors (3‑D)
    float vecX[pairCount][3];
    float vecY[pairCount][3];

    // Deterministically generate a wide variety of edge‑case patterns
    // Cycle through the original six cases for each successive pair
    for (int i = 0; i < pairCount; ++i) {
        int pattern = i % 6;
        switch (pattern) {
            case 0: // Both zero vectors
                vecX[i][0] = 0; vecX[i][1] = 0; vecX[i][2] = 0;
                vecY[i][0] = 0; vecY[i][1] = 0; vecY[i][2] = 0;
                break;
            case 1: // One zero, one non‑zero
                vecX[i][0] = 0; vecX[i][1] = 0; vecX[i][2] = 0;
                vecY[i][0] = 5; vecY[i][1] = -3; vecY[i][2] = 2;
                break;
            case 2: // Parallel (same direction)
                vecX[i][0] = 1; vecX[i][1] = 2; vecX[i][2] = 3;
                vecY[i][0] = 2; vecY[i][1] = 4; vecY[i][2] = 6;
                break;
            case 3: // Anti‑parallel (opposite direction)
                vecX[i][0] = -1; vecX[i][1] = -2; vecX[i][2] = -3;
                vecY[i][0] = 1;  vecY[i][1] = 2;  vecY[i][2] = 3;
                break;
            case 4: // Orthogonal
                vecX[i][0] = 1; vecX[i][1] = 0; vecX[i][2] = 0;
                vecY[i][0] = 0; vecY[i][1] = 5; vecY[i][2] = 0;
                break;
            case 5: // Large mixed values
                vecX[i][0] = 1e4f; vecX[i][1] = -2e4f; vecX[i][2] = 3e4f;
                vecY[i][0] = -4e4f; vecY[i][1] = 5e4f; vecY[i][2] = -6e4f;
                break;
        }
    }

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
