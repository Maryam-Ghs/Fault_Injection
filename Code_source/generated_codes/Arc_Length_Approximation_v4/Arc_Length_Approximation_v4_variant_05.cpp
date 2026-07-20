#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>

/* LLM input variant 5: duplicate-heavy */

int main() {
    // Seed the random number generator (kept for consistency, though not used)
    std::srand(static_cast<int>(std::time(0)));

    // -----------------------------------------------------------------
    // 1️⃣  Define the size of the data set (medium‑sized array)
    // -----------------------------------------------------------------
    int pointsCount = 12;                 // number of sample points

    // -------------------------------------------------
    // 2️⃣  Allocate stack arrays for the coordinates
    // -------------------------------------------------
    float arrX[12];                       // x‑coordinates
    float arrY[12];                       // y‑coordinates

    // -------------------------------------------------
    // 3️⃣  Fill arrX with a duplicate‑heavy deterministic pattern
    // -------------------------------------------------
    // Pattern: each integer value appears three times consecutively
    // Values: 1.0, 1.0, 1.0, 2.0, 2.0, 2.0, 3.0, 3.0, 3.0, 4.0, 4.0, 4.0
    for (int i = 0; i < pointsCount; ++i) {
        arrX[i] = static_cast<float>((i / 3) + 1); // integer division creates groups of three
    }

    // -------------------------------------------------
    // 4️⃣  Sort arrX so the points form a proper function curve
    // -------------------------------------------------
    int outer = 0;
    while (outer < pointsCount - 1) {
        int inner = 0;
        while (inner < pointsCount - outer - 1) {
            if (arrX[inner] > arrX[inner + 1]) {
                float tmp = arrX[inner];
                arrX[inner] = arrX[inner + 1];
                arrX[inner + 1] = tmp;
            }
            ++inner;
        }
        ++outer;
    }

    // -------------------------------------------------
    // 5️⃣  Compute y = x² for each x (parabolic curve)
    // -------------------------------------------------
    for (int idx = 0; idx < pointsCount; ++idx) {
        float xVal = arrX[idx];
        arrY[idx] = xVal * xVal;
    }

    // -------------------------------------------------
    // 6️⃣  Approximate the arc length by summing segment lengths
    // -------------------------------------------------
    float totalLen = 0.0f;
    for (int idx = 1; idx < pointsCount; ++idx) {
        // Δx and Δy
        float deltaX = arrX[idx] - arrX[idx - 1];
        float deltaY = arrY[idx] - arrY[idx - 1];

        // Squares of the deltas
        float sqX = deltaX * deltaX;
        float sqY = deltaY * deltaY;

        // Sum of squares
        float sumSq = sqX + sqY;

        // Length of this segment (√(Δx²+Δy²))
        float segLen = sqrtf(sumSq);

        // Accumulate
        totalLen = totalLen + segLen;
    }

    // -------------------------------------------------
    // 7️⃣  Print the result
    // -------------------------------------------------
    std::cout << "Number of points : " << pointsCount << '\n';
    std::cout << "x‑range           : [" << arrX[0] << " , " << arrX[pointsCount - 1] << "]\n";
    std::cout << "Approximate arc length of y = x^2 : " << totalLen << '\n';

    return 0;
}
