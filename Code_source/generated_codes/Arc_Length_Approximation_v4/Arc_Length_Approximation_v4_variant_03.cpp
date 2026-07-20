#include <iostream>
#include <cmath>

/* LLM input variant 3: zeros-and-ones */

int main() {
    // -----------------------------------------------------------------
    // 1️⃣  Define the size of the data set (focus on zeros and ones)
    // -----------------------------------------------------------------
    int pointsCount = 8;                 // number of sample points (emphasizing 0 and 1)

    // -------------------------------------------------
    // 2️⃣  Allocate stack arrays for the coordinates
    // -------------------------------------------------
    float arrX[12];                       // x‑coordinates (size >= pointsCount)
    float arrY[12];                       // y‑coordinates

    // -------------------------------------------------
    // 3️⃣  Fill arrX with deterministic values emphasizing zeros and ones
    // -------------------------------------------------
    // Values: three zeros followed by five ones (already sorted)
    float presetX[8] = {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    for (int i = 0; i < pointsCount; ++i) {
        arrX[i] = presetX[i];
    }

    // -------------------------------------------------
    // 4️⃣  (Sorting step retained but unnecessary for preset data)
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
        float deltaX = arrX[idx] - arrX[idx - 1];
        float deltaY = arrY[idx] - arrY[idx - 1];
        float segLen = sqrtf(deltaX * deltaX + deltaY * deltaY);
        totalLen += segLen;
    }

    // -------------------------------------------------
    // 7️⃣  Print the result
    // -------------------------------------------------
    std::cout << "Number of points : " << pointsCount << '\n';
    std::cout << "x‑range           : [" << arrX[0] << " , " << arrX[pointsCount - 1] << "]\n";
    std::cout << "Approximate arc length of y = x^2 : " << totalLen << '\n';

    return 0;
}
