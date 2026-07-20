/* LLM input variant 8: sparse-skewed */

// Arc Length Approximation – version #5
// All code resides inside main(), uses only float/int, heap allocation, and no input.

#include <iostream>
#include <cmath>

int main() {
    // Number of sample points (increased to create sparsity)
    int aCount = 20;

    // Allocate coordinate arrays on the heap
    float* xVals = new float[aCount];
    float* yVals = new float[aCount];

    // Initialize all entries to zero (mostly empty)
    for (int i = 0; i < aCount; ++i) {
        xVals[i] = 0.0f;
        yVals[i] = 0.0f;
    }

    // Populate a few sparse, clustered points following y = x^2
    xVals[0]  = 0.0f;   yVals[0]  = 0.0f;    // start
    xVals[5]  = 5.0f;   yVals[5]  = 25.0f;   // cluster 1
    xVals[10] = 10.0f;  yVals[10] = 100.0f;  // cluster 2
    xVals[15] = 15.0f;  yVals[15] = 225.0f;  // cluster 3
    xVals[19] = 19.0f;  yVals[19] = 361.0f;  // end

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
