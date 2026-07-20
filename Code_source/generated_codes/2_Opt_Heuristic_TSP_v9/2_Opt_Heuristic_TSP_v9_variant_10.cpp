/*********************************************************************
 * 2‑Opt Heuristic for the Traveling Salesman Problem
 * Version #9 – Verbose, stack‑based, class implementation
 *
 * Constraints:
 *  - Only int and float types are used (no double, long, unsigned, const)
 *  - Input is generated internally with edge‑case heavy data
 *  - All data lives on the stack (fixed‑size arrays)
 *  - No use of std::cin; results are printed to stdout
 *********************************************************************/

#include <cstdio>
#include <cmath>
#include <algorithm>   // for std::swap only (no dynamic containers)

// -------------------------------------------------------------------
// Configuration (stack‑based, fixed size)
// -------------------------------------------------------------------
#define MAX_CITY 100          // maximum number of cities handled

/* LLM input variant 10: large-safe-stress */

// -------------------------------------------------------------------
// Helper class that encapsulates the whole 2‑Opt process
// -------------------------------------------------------------------
class TSPSolver2Opt {
public:
    // ----------------------------------------------------------------
    // Public interface
    // ----------------------------------------------------------------
    void run() {
        generateEdgeCaseCities();
        computeDistanceMatrix();
        buildInitialTour();
        improveTourWithTwoOpt();
        printResult();
    }

private:
    // ----------------------------------------------------------------
    // Data members (all allocated on the stack)
    // ----------------------------------------------------------------
    int cityCount;                          // actual number of cities
    int cityX[MAX_CITY];                    // x‑coordinates
    int cityY[MAX_CITY];                    // y‑coordinates
    float matrixDist[MAX_CITY][MAX_CITY];   // Euclidean distances
    int order[MAX_CITY + 1];                // current tour (last element repeats start)

    // ----------------------------------------------------------------
    // 1. Generate a set of cities that stress the algorithm
    //    – duplicate points, collinear points, extreme coordinates
    // ----------------------------------------------------------------
    void generateEdgeCaseCities() {
        cityCount = MAX_CITY;               // using the maximum for the test

        // Duplicate points (city 0 and 1 are identical)
        cityX[0] = 0;   cityY[0] = 0;
        cityX[1] = 0;   cityY[1] = 0;

        // Collinear points along the X‑axis (cities 2‑11)
        for (int i = 2; i < 12; ++i) {
            cityX[i] = (i - 2) * 10;   // 0,10,20,...
            cityY[i] = 0;
        }

        // Points forming a large square (cities 12‑15)
        cityX[12] = -1000; cityY[12] = -1000;
        cityX[13] = -1000; cityY[13] =  1000;
        cityX[14] =  1000; cityY[14] =  1000;
        cityX[15] =  1000; cityY[15] = -1000;

        // Remaining interior points (cities 16‑99) generated with a simple pattern
        for (int i = 16; i < MAX_CITY; ++i) {
            // Produce pseudo‑random coordinates within [-1000, 1000]
            cityX[i] = ((i * 123) % 2001) - 1000;   // modulus ensures range
            cityY[i] = ((i * 321) % 2001) - 1000;
        }
    }

    // ----------------------------------------------------------------
    // 2. Compute the full Euclidean distance matrix (float, sqrtf)
    // ----------------------------------------------------------------
    void computeDistanceMatrix() {
        int i = 0;
        while (i < cityCount) {
            int j = 0;
            while (j < cityCount) {
                int dx = cityX[i] - cityX[j];
                int dy = cityY[i] - cityY[j];
                // sqrtf returns a float, fulfilling the type restriction
                matrixDist[i][j] = sqrtf(float(dx * dx + dy * dy));
                ++j;
            }
            ++i;
        }
    }

    // ----------------------------------------------------------------
    // 3. Build an initial naive tour (0 → 1 → 2 → … → N‑1 → 0)
    // ----------------------------------------------------------------
    void buildInitialTour() {
        int k = 0;
        while (k < cityCount) {
            order[k] = k;
            ++k;
        }
        order[cityCount] = 0;   // close the loop
    }

    // ----------------------------------------------------------------
    // 4. Core 2‑Opt improvement loop
    // ----------------------------------------------------------------
    void improveTourWithTwoOpt() {
        bool anyImprovement = true;

        // Continue until a full pass yields no improvement
        while (anyImprovement) {
            anyImprovement = false;

            // i loops over the first cut point (skip the depot at position 0)
            int i = 1;
            while (i < cityCount - 1) {
                // j loops over the second cut point (must be at least two steps ahead)
                int j = i + 1;
                while (j < cityCount) {
                    // Compute the gain if we would reverse the segment (i … j)
                    float before = matrixDist[order[i - 1]][order[i]]
                                 + matrixDist[order[j]][order[(j + 1) % cityCount]];
                    float after  = matrixDist[order[i - 1]][order[j]]
                                 + matrixDist[order[i]][order[(j + 1) % cityCount]];
                    float gain = before - after;   // positive gain means improvement

                    // If we found a beneficial swap, perform the reversal
                    if (gain > 0.0001f) {
                        reverseSegment(i, j);
                        anyImprovement = true;
                    }
                    ++j;
                }
                ++i;
            }
        }
    }

    // ----------------------------------------------------------------
    // Helper: reverse the order between indices a and b (inclusive)
    // ----------------------------------------------------------------
    void reverseSegment(int a, int b) {
        // Classic in‑place reversal
        while (a < b) {
            // swap order[a] and order[b] without using std::swap to keep it explicit
            int tmp = order[a];
            order[a] = order[b];
            order[b] = tmp;
            ++a;
            --b;
        }
    }

    // ----------------------------------------------------------------
    // 5. Compute total length of the final tour and print everything
    // ----------------------------------------------------------------
    void printResult() {
        // Compute total distance
        float totalLen = 0.0f;
        int idx = 0;
        while (idx < cityCount) {
            totalLen += matrixDist[order[idx]][order[idx + 1]];
            ++idx;
        }

        // Header
        std::printf("2‑Opt TSP Solver – Version #9\n");
        std::printf("Number of cities : %d\n", cityCount);
        std::printf("Final tour length: %.2f\n", totalLen);
        std::printf("Tour order (city indices):\n");

        // Print the tour sequence
        int p = 0;
        while (p <= cityCount) {
            std::printf("%d ", order[p]);
            ++p;
        }
        std::printf("\n");
    }
};

// -------------------------------------------------------------------
// Main entry point – no external input, everything is self‑contained
// -------------------------------------------------------------------
int main() {
    TSPSolver2Opt solver;
    solver.run();
    return 0;
}
