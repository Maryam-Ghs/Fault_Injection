#include <iostream>
#include <cstdlib>
#include <ctime>

/* LLM input variant 9: medium-deterministic-random */

const int MAX_N = 20;          // maximum number of circles

void populateRadii(float radii[], int count) {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    for (int i = 0; i < count; ++i) {
        radii[i] = static_cast<float>((std::rand() % 10) + 1);
    }
}

float lineLength(const float rad[], int n) {
    float curPos = rad[0];
    float rightEdge = curPos + rad[0];
    float gap, newCenter, newRight;
    for (int idx = 1; idx < n; ++idx) {
        gap = rad[idx - 1] + rad[idx];
        newCenter = curPos + gap;
        newRight = newCenter + rad[idx];
        curPos = newCenter;
        rightEdge = newRight;
    }
    return rightEdge;
}

int factorial(int value) {
    int result = 1;
    for (int i = 2; i <= value; ++i) result *= i;
    return result;
}

int main() {
    const int circleCount = 12;                // medium size, safe for int factorial
    float radii[MAX_N];

    // deterministic pseudo‑random‑looking sequence
    float inputVals[] = { 4.0f, 7.0f, 2.0f, 9.0f, 5.0f, 3.0f,
                          8.0f, 1.0f, 6.0f,10.0f, 2.0f, 5.0f };
    for (int i = 0; i < circleCount; ++i) radii[i] = inputVals[i];

    float neededLength = lineLength(radii, circleCount);
    int arrangements = factorial(circleCount);

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
