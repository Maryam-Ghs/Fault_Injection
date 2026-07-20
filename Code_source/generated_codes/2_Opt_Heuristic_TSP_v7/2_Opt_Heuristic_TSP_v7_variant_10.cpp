#include <iostream>
#include <cmath>
#include <vector>

/* LLM input variant 10: large-safe-stress */

int main() {
    // ---------- generate a larger TSP instance ----------
    const int cityCount = 2000;               // number of cities (including the start)
    int *coordX = new int[cityCount];
    int *coordY = new int[cityCount];

    // deterministic pseudo‑random coordinates within safe range
    for (int i = 0; i < cityCount; ++i) {
        coordX[i] = (i * 7) % 1000;            // x in [0, 999]
        coordY[i] = (i * 13) % 1000;           // y in [0, 999]
    }

    // ---------- allocate tour on the heap ----------
    int *order = new int[cityCount + 1];       // last element repeats the start
    for (int i = 0; i < cityCount; ++i) order[i] = i;
    order[cityCount] = 0;                     // close the loop

    // ---------- helper lambdas ----------
    auto distBetween = [&](int p, int q) -> float {
        int dx = coordX[p] - coordX[q];
        int dy = coordY[p] - coordY[q];
        float sq = static_cast<float>(dx * dx + dy * dy);
        return std::sqrt(sq);
    };

    auto tourLength = [&]() -> float {
        float total = 0.0f;
        for (int i = 0; i < cityCount; ++i) {
            total += distBetween(order[i], order[i + 1]);
        }
        return total;
    };

    // ---------- 2‑Opt improvement ----------
    auto twoOptStep = [&]() -> bool {
        bool anyImprovement = false;
        for (int start = 1; start < cityCount - 1; ++start) {
            for (int end = start + 1; end < cityCount; ++end) {
                int a = order[start - 1];
                int b = order[start];
                int c = order[end];
                int d = order[end + 1];

                float curAB = distBetween(a, b);
                float curCD = distBetween(c, d);
                float newAC = distBetween(a, c);
                float newBD = distBetween(b, d);

                float gain = (curAB + curCD) - (newAC + newBD);
                if (gain > 0.0f) {
                    int left = start, right = end;
                    while (left < right) {
                        int tmp = order[left];
                        order[left] = order[right];
                        order[right] = tmp;
                        ++left;
                        --right;
                    }
                    anyImprovement = true;
                }
            }
        }
        return anyImprovement;
    };

    // ---------- main 2‑Opt loop ----------
    bool improved = true;
    while (improved) {
        improved = twoOptStep();
    }

    // ---------- output ----------
    std::cout << "Final tour order:\n";
    for (int i = 0; i <= cityCount; ++i) {
        std::cout << order[i] << (i == cityCount ? '\n' : ' ');
    }
    std::cout << "Tour length: " << tourLength() << "\n";

    // ---------- clean up ----------
    delete [] coordX;
    delete [] coordY;
    delete [] order;

    return 0;
}
