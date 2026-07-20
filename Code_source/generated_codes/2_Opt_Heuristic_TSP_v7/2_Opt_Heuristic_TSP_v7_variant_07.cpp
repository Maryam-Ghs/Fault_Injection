/* LLM input variant 7: reverse-adversarial */
#include <iostream>
#include <cmath>
#include <vector>

int main() {
    // ---------- generate an adversarial TSP instance ----------
    int cityCount = 10;                         // number of cities (including the start)
    int *coordX = new int[cityCount];
    int *coordY = new int[cityCount];

    // coordinates forming a grid with a distant outlier
    coordX[0] =   0; coordY[0] =   0;
    coordX[1] =   0; coordY[1] =  10;
    coordX[2] =   0; coordY[2] =  20;
    coordX[3] =  10; coordY[3] =   0;
    coordX[4] =  10; coordY[4] =  10;
    coordX[5] =  10; coordY[5] =  20;
    coordX[6] =  20; coordY[6] =   0;
    coordX[7] =  20; coordY[7] =  10;
    coordX[8] =  20; coordY[8] =  20;
    coordX[9] =  30; coordY[9] =  10; // outlier to create crossing possibilities

    // ---------- allocate tour on the heap ----------
    int *order = new int[cityCount + 1];       // last element repeats the start
    order[0] = 0;                             // start city
    for (int i = 1; i < cityCount; ++i) order[i] = cityCount - i; // reverse order for adversarial start
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
