#include <iostream>
#include <cstdlib>
#include <ctime>

/* LLM input variant 1: minimal-boundary */

const int MAX_N = 20;          // maximum number of circles

// -----------------------------------------------------------------
// Fill the array with pseudo‑random radii (float values 1 … 10)
// -----------------------------------------------------------------
void populateRadii(float radii[], int count) {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    for (int i = 0; i < count; ++i) {
        // generate a random integer 1‑10 and cast to float
        radii[i] = static_cast<float>((std::rand() % 10) + 1);
    }
}

// -----------------------------------------------------------------
// Compute the total length of a straight line that can hold all
// circles when each touches its neighbour (centres are aligned).
// -----------------------------------------------------------------
float lineLength(const float rad[], int n) {
    // position of the first centre – we start at its left edge = 0
    float curPos = rad[0];
    // rightmost edge after placing the first circle
    float rightEdge = curPos + rad[0];

    // temporary variables used inside the loop
    float gap, newCenter, newRight;

    for (int idx = 1; idx < n; ++idx) {
        gap       = rad[idx - 1] + rad[idx];   // centre‑to‑centre distance
        newCenter = curPos + gap;              // centre of current circle
        newRight  = newCenter + rad[idx];      // its right edge

        // update for next iteration
        curPos   = newCenter;
        rightEdge = newRight;
    }
    return rightEdge;   // left edge is 0, so length == rightmost edge
}

// -----------------------------------------------------------------
// Compute factorial (n!) using only int arithmetic.
// -----------------------------------------------------------------
int factorial(int value) {
    int result = 1;
    for (int i = 2; i <= value; ++i) {
        result *= i;
    }
    return result;
}

// -----------------------------------------------------------------
int main() {
    // -----------------------------------------------------------------
    // 1. generate a random sized problem (medium size)
    // -----------------------------------------------------------------
    const int circleCount = 1;                 // minimal non‑trivial size
    float radii[MAX_N];                         // stack‑allocated array

    // deterministic minimal input
    float inputVals[] = { 1.0f };
    for (int i = 0; i < circleCount; ++i) radii[i] = inputVals[i];

    // -----------------------------------------------------------------
    // 2. compute the required line length
    // -----------------------------------------------------------------
    float neededLength = lineLength(radii, circleCount);

    // -----------------------------------------------------------------
    // 3. compute the number of possible orderings (arrangements)
    // -----------------------------------------------------------------
    int arrangements = factorial(circleCount);

    // -----------------------------------------------------------------
    // 4. output the data
    // -----------------------------------------------------------------
    std::cout << "Generated radii (" << circleCount << " circles):\n";
    for (int i = 0; i < circleCount; ++i) {
        std::cout << radii[i] << (i + 1 == circleCount ? '\n' : ' ');
    }

    std::cout << "Total line length needed to place them touching each other: "
              << neededLength << "\n";

    std::cout << "Number of different linear arrangements (n!): "
              << arrangements << "\n";

    return 0;
}
