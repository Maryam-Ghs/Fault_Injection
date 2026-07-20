/* LLM input variant 7: reverse-adversarial */
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>

int main() {
    // Seed the random number generator (no unsigned keyword used)
    std::srand(static_cast<int>(std::time(0)));

    // -----------------------------------------------------------------
    // 1️⃣  Define the size of the data set (medium‑sized reverse‑ordered array)
    // -----------------------------------------------------------------
    int pointsCount = 12;                 // number of sample points

    // -------------------------------------------------
    // 2️⃣  Allocate stack arrays for the coordinates
    // -------------------------------------------------
    float arrX[12];                       // x‑coordinates
    float arrY[12];                       // y‑coordinates

    // -------------------------------------------------
    // 3️⃣  Fill arrX with deterministic descending values in the interval [0,10]
    // -------------------------------------------------
    int idx = 0;
    while (idx < pointsCount) {
        // Create a descending sequence: start at 10, end at 0
        arrX[idx] = static_cast<float>(pointsCount - idx - 1) * (10.0f / static_cast<float>(pointsCount - 1));
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
