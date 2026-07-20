/* LLM input variant 4: signed-extremes */
// Arc Length Approximation – version #5
// All code resides inside main(), uses only float/int, heap allocation, and no input.

#include <iostream>
#include <cmath>

int main() {
    // Number of sample points (small predefined array)
    int aCount = 5;

    // Allocate coordinate arrays on the heap
    float* xVals = new float[aCount];
    float* yVals = new float[aCount];

    // Fill the arrays with a cubic curve y = x^3, mixing negative, zero, and positive values
    xVals[0] = -2.0f;  yVals[0] = -8.0f;
    xVals[1] = -1.0f;  yVals[1] = -1.0f;
    xVals[2] =  0.0f;  yVals[2] =  0.0f;
    xVals[3] =  1.0f;  yVals[3] =  1.0f;
    xVals[4] =  2.0f;  yVals[4] =  8.0f;

    // Approximate the arc length by summing the lengths of straight‑line
    // segments between successive points:  L ≈ Σ sqrt( (Δx)² + (Δy)² )
    float totalLen = 0.0f;
    int idx = 1;                     // start from the second point
    while (idx < aCount) {
        // Step 1: compute differences
        float deltaX = xVals[idx] - xVals[idx - 1];
        float deltaY = yVals[idx] - yVals[idx - 1];

        // Step 2: square the differences
        float sqDeltaX = deltaX * deltaX;
        float sqDeltaY = deltaY * deltaY;

        // Step 3: add the squares
        float sumSquares = sqDeltaX + sqDeltaY;

        // Step 4: take the square root (segment length)
        float segLen = sqrtf(sumSquares);

        // Step 5: accumulate
        totalLen = totalLen + segLen;

        // Move to the next segment
        idx = idx + 1;
    }

    // Output the result
    std::cout << "Approximate arc length: " << totalLen << std::endl;

    // Clean up heap memory
    delete[] xVals;
    delete[] yVals;

    return 0;
}
