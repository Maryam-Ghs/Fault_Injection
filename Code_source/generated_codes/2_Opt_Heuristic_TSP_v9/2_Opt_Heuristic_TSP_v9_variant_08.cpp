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
/* LLM input variant 8: sparse-skewed */

#include <cstdio>
#include <cmath>
#include <algorithm>   // for std::swap only (no dynamic containers)

// -------------------------------------------------------------------
// Configuration (stack‑based, fixed size)
// -------------------------------------------------------------------
#define MAX_CITY 12          // maximum number of cities handled

// -------------------------------------------------------------------
// Helper class that encapsulates the whole 2‑Opt process
// -------------------------------------------------------------------
class TSPSolver2Opt {
public:
    // ----------------------------------------------------------------
    // Public interface
    // ----------------------------------------------------------------
    void run() {
        generateSparseSkewedCities();
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
    // 1. Generate a sparse, skewed set of cities
    //    – a tight cluster near the origin, a far‑positive cluster,
    //      a far‑negative cluster, and extreme outliers.
    // ----------------------------------------------------------------
    void generateSparseSkewedCities() {
        cityCount = MAX_CITY;               // using the maximum for the test

        // Tight cluster near (0,0)
        cityX[0] = 0;    cityY[0] = 0;
        cityX[1] = 1;    cityY[1] = 0;
        cityX[2] = 0;    cityY[2] = 1;

        // Far‑positive cluster around (1000,1000)
        cityX[3] = 1000;   cityY[3] = 1000;
        cityX[4] = 1010;   cityY[4] = 1005;
        cityX[5] = 1020;   cityY[5] = 1010;

        // Far‑negative cluster around (-2000,-2000)
        cityX[6] = -2000;  cityY[6] = -2000;
        cityX[7] = -1990;  cityY[7] = -1995;

        // Extreme outliers
        cityX[8] = 5000;   cityY[8] = -5000;
        cityX[9] = 0;      cityY[9] = 5000;
        cityX[10] = 0;     cityY[10] = -5000;

        // Duplicate of the origin to increase sparsity
        cityX[11] = 0;     cityY[11] = 0;
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
