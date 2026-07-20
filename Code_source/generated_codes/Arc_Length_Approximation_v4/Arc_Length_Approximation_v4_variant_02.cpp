#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>

/* LLM input variant 2: small-diverse */

int main() {
    // Seed the random number generator (no unsigned keyword used)
    std::srand(static_cast<int>(std::time(0)));

    // -----------------------------------------------------------------
    // 1️⃣  Define the size of the data set (small diverse deterministic array)
    // -----------------------------------------------------------------
    int pointsCount = 7;                 // number of sample points

    // -------------------------------------------------
    // 2️⃣  Allocate stack arrays for the coordinates
    // -------------------------------------------------
    float arrX[7];                       // x‑coordinates
    float arrY[7];                       // y‑coordinates

    // -------------------------------------------------
    // 3️⃣  Fill arrX with predetermined diverse values
    // -------------------------------------------------
    const float presetX[7] = {8.5f, 2.1f, 5.0f, 9.3f, 1.2f, 6.7f, 3.4f};
    int idx = 0;
    while (idx < pointsCount) {
        arrX[idx] = presetX[idx];
        ++idx;
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
    idx = 0;
    while (idx < pointsCount) {
        float xVal = arrX[idx];
        arrY[idx] = xVal * xVal;
        ++idx;
    }

    // -------------------------------------------------
    // 6️⃣  Approximate the arc length by summing segment lengths
    // -------------------------------------------------
    float totalLen = 0.0f;
    idx = 1;
    while (idx < pointsCount) {
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

        ++idx;
    }

    // -------------------------------------------------
    // 7️⃣  Print the result
    // -------------------------------------------------
    std::cout << "Number of points : " << pointsCount << '\n';
    std::cout << "x‑range           : [" << arrX[0] << " , " << arrX[pointsCount - 1] << "]\n";
    std::cout << "Approximate arc length of y = x^2 : " << totalLen << '\n';

    return 0;
}
