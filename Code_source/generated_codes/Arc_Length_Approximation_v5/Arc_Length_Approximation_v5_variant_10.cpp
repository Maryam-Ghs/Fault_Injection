/* LLM input variant 10: large-safe-stress */
// Arc Length Approximation – version #5
// All code resides inside main(), uses only float/int, heap allocation, and no input.

#include <iostream>
#include <cmath>

int main() {
    // Number of sample points (large but safe)
    int aCount = 10000;

    // Allocate coordinate arrays on the heap
    float* xVals = new float[aCount];
    float* yVals = new float[aCount];

    // Fill the arrays with a quadratic curve y = x^2 using a small step
    const float step = 0.001f; // ensures values stay within reasonable range
    for (int i = 0; i < aCount; ++i) {
        xVals[i] = i * step;
        yVals[i] = xVals[i] * xVals[i];
    }

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
