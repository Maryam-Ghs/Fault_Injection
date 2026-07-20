/ * LLM input variant 6: ordered-structured * /
#include <iostream>
#include <cmath>

int main() {
    // Number of sample points (symmetrically ordered array)
    int aCount = 9;

    // Allocate coordinate arrays on the heap
    float* xVals = new float[aCount];
    float* yVals = new float[aCount];

    // Fill the arrays with a symmetric quadratic curve y = x^2,
    // x ranging from -4 to 4 (inclusive) in unit steps
    for (int i = 0; i < aCount; ++i) {
        xVals[i] = static_cast<float>(i - 4);          // -4, -3, ..., 4
        yVals[i] = xVals[i] * xVals[i];               // square of x
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
