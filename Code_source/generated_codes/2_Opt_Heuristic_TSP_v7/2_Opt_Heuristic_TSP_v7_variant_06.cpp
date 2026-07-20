#include <iostream>
#include <cmath>
#include <vector>

/* LLM input variant 6: ordered-structured */

int main() {
    // ---------- generate a structured TSP instance ----------
    int cityCount = 8;                         // number of cities (including the start)
    int *coordX = new int[cityCount];
    int *coordY = new int[cityCount];

    // ordered coordinates along the x‑axis, y = 0
    for (int i = 0; i < cityCount; ++i) {
        coordX[i] = i * 10;
        coordY[i] = 0;
    }

    // ---------- allocate tour on the heap ----------
    int *order = new int[cityCount + 1];       // last element repeats the start
    for (int i = 0; i < cityCount; ++i) order[i] = i;
    order[cityCount] = 0;                      // close the loop

    // ---------- helper lambdas (function‑based modular) ----------
    auto distBetween = [&](int p, int q) -> float {
        int dx = coordX[p] - coordX[q];
        int dy = coordY[p] - coordY[q];
        float sq = static_cast<float>(dx * dx + dy * dy);
        return std::sqrt(sq);
    };

    auto tourLength = [&]() -> float {
        float total = 0.0f;
        for (int i = 0; i < cityCount; ++i) {
            float part = distBetween(order[i], order[i + 1]);
            total += part;
        }
        return total;
    };

    // ---------- 2‑Opt improvement (math split into temporaries) ----------
    auto twoOptStep = [&]() -> bool {
        bool anyImprovement = false;
        for (int start = 1; start < cityCount - 1; ++start) {
            for (int end = start + 1; end < cityCount; ++end) {
                // edges before the reversal
                int a = order[start - 1];
                int b = order[start];
                int c = order[end];
                int d = order[end + 1];

                // distances of current edges
                float curAB = distBetween(a, b);
                float curCD = distBetween(c, d);

                // distances if we flip the segment
                float newAC = distBetween(a, c);
                float newBD = distBetween(b, d);

                // gain computation
                float gain = (curAB + curCD) - (newAC + newBD);

                if (gain > 0.0f) {
                    // reverse the segment [start, end]
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
