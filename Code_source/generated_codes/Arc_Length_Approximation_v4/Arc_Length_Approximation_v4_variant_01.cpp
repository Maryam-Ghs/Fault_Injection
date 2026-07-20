#include <iostream>
#include <cmath>

/* LLM input variant 1: minimal-boundary */

int main() {
    // 1️⃣ Define the size of the data set (minimal nontrivial)
    int pointsCount = 2;                 // number of sample points

    // 2️⃣ Allocate stack arrays for the coordinates
    float arrX[2];                       // x‑coordinates
    float arrY[2];                       // y‑coordinates

    // 3️⃣ Fill arrX with deterministic boundary‑valid values in [0,10)
    arrX[0] = 0.0f;                      // lower bound
    arrX[1] = 9.9f;                      // just below upper bound

    // 4️⃣ Sort arrX (already sorted, but keep algorithm)
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

    // 5️⃣ Compute y = x² for each x (parabolic curve)
    for (int idx = 0; idx < pointsCount; ++idx) {
        float xVal = arrX[idx];
        arrY[idx] = xVal * xVal;
    }

    // 6️⃣ Approximate the arc length by summing segment lengths
    float totalLen = 0.0f;
    for (int idx = 1; idx < pointsCount; ++idx) {
        float deltaX = arrX[idx] - arrX[idx - 1];
        float deltaY = arrY[idx] - arrY[idx - 1];
        float segLen = sqrtf(deltaX * deltaX + deltaY * deltaY);
        totalLen += segLen;
    }

    // 7️⃣ Print the result
    std::cout << "Number of points : " << pointsCount << '\n';
    std::cout << "x‑range           : [" << arrX[0] << " , " << arrX[pointsCount - 1] << "]\n";
    std::cout << "Approximate arc length of y = x^2 : " << totalLen << '\n';

    return 0;
}
