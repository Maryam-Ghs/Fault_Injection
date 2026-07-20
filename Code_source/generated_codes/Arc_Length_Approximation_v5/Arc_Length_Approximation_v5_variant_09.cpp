/* LLM input variant 9: medium-deterministic-random */
// Arc Length Approximation – version #5
// All code resides inside main(), uses only float/int, heap allocation, and no input.

#include <iostream>
#include <cmath>
#include <cstdint>

int main() {
    // Number of sample points (medium-sized deterministic pseudo‑random data)
    const int aCount = 12;

    // Allocate coordinate arrays on the heap
    float* xVals = new float[aCount];
    float* yVals = new float[aCount];

    // Fill the arrays with deterministic pseudo‑random values using a simple LCG
    uint32_t seed = 123456789u;
    auto next = [&]() -> uint32_t {
        seed = seed * 1664525u + 1013904223u;
        return seed;
    };

    for (int i = 0; i < aCount; ++i) {
        float x = static_cast<float>((next() % 1000)) / 10.0f;  // 0.0 … 99.9
        float y = static_cast<float>((next() % 1000)) / 10.0f;  // 0.0 … 99.9
        xVals[i] = x;
        yVals[i] = y;
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
