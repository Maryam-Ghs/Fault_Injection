// 2‑Opt Heuristic for TSP – version #6
// ---------------------------------------------------------------
// Implements a simple deterministic test case, uses only int/float,
// heap allocation, and helper functions.  The code is written with
// a branch‑minimized style and reordered arithmetic where possible.
// ---------------------------------------------------------------

/* LLM input variant 9: medium-deterministic-random */

#include <iostream>
#include <cmath>

// ---------------------------------------------------------------
// Simple point structure (no const, no double)
struct City {
    float x;
    float y;
};

// ---------------------------------------------------------------
// Euclidean distance (float only)
float dist(const City* a, const City* b) {
    float dx = a->x - b->x;
    float dy = a->y - b->y;
    // reordered: sum of squares first, sqrt later
    return sqrtf(dx * dx + dy * dy);
}

// ---------------------------------------------------------------
// Compute total length of a tour
float tourLength(const City* pts, const int* tour, int n) {
    float sum = 0.0f;
    for (int i = 0; i < n; ++i) {
        const City* cur = pts + tour[i];
        const City* nxt = pts + tour[(i + 1) % n];
        sum += dist(cur, nxt);
    }
    return sum;
}

// ---------------------------------------------------------------
// Reverse the segment (i+1 … k) in‑place
void reverseSegment(int* tour, int i, int k) {
    while (i < k) {
        int tmp = tour[i];
        tour[i] = tour[k];
        tour[k] = tmp;
        ++i;
        --k;
    }
}

// ---------------------------------------------------------------
// Single 2‑Opt iteration – returns true if an improvement was made
bool twoOptIteration(const City* pts, int* tour, int n) {
    bool improved = false;
    // i runs from 1 to n‑2 (skip the first city to keep start fixed)
    for (int i = 1; i < n - 2 && !improved; ++i) {
        const City* a = pts + tour[i - 1];
        const City* b = pts + tour[i];
        // k runs from i+1 to n‑1 (last edge closes the tour)
        for (int k = i + 1; k < n && !improved; ++k) {
            const City* c = pts + tour[k];
            const City* d = pts + tour[(k + 1) % n];

            // gain = (ab + cd) - (ac + bd); positive gain = improvement
            float gain = (dist(a, b) + dist(c, d)) - (dist(a, c) + dist(b, d));
            // branch‑minimized: evaluate as bool then act
            improved = gain > 0.0f;
            // if improvement, perform the 2‑Opt swap
            improved && (reverseSegment(tour, i, k), 0);
        }
    }
    return improved;
}

// ---------------------------------------------------------------
// Full 2‑Opt heuristic loop
void runTwoOpt(const City* pts, int* tour, int n) {
    // repeat until no improvement
    while (twoOptIteration(pts, tour, n)) {
        // empty body – work done in twoOptIteration
    }
}

// ---------------------------------------------------------------
// Main – deterministic test vector, heap allocation, result output
int main() {
    // ----- deterministic test data (12 cities) -----
    int cityCount = 12;
    City* points = new City[cityCount];
    // Pseudo‑random‑looking coordinates generated with a simple LCG formula
    // x = (13*i*i + 7) % 100 + 5, y = (23*i + 11) % 100 + 10
    for (int i = 0; i < cityCount; ++i) {
        float x = static_cast<float>((13 * i * i + 7) % 100 + 5);
        float y = static_cast<float>((23 * i + 11) % 100 + 10);
        points[i] = { x, y };
    }

    // initial tour: 0,1,2,…,n‑1
    int* order = new int[cityCount];
    for (int i = 0; i < cityCount; ++i) order[i] = i;

    // run the heuristic
    runTwoOpt(points, order, cityCount);

    // compute final length
    float finalLen = tourLength(points, order, cityCount);

    // ----- output -----
    std::cout << "Final tour length: " << finalLen << "\n";
    std::cout << "Tour order: ";
    for (int i = 0; i < cityCount; ++i) {
        std::cout << order[i] << (i + 1 == cityCount ? "\n" : " ");
    }

    // clean up
    delete[] points;
    delete[] order;
    return 0;
}
