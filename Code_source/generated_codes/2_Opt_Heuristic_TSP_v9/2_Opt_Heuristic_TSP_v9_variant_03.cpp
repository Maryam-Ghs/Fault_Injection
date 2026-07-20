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

// LLM input variant 3: zeros-and-ones

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
    //    – duplicate points, minimal coordinates, identity values
    // ----------------------------------------------------------------
    void generateEdgeCaseCities() {
        cityCount = MAX_CITY;               // using the maximum for the test

        // Duplicate points (city 0 and 1 are identical at origin)
        cityX[0] = 0;   cityY[0] = 0;
        cityX[1] = 0;   cityY[1] = 0;

        // Points using only 0 and 1 coordinates
        cityX[2] = 1;   cityY[2] = 0;
        cityX[3] = 0;   cityY[3] = 1;
        cityX[4] = 1;   cityY[4] = 1;

        // More duplicate and identity‑like points
        cityX[5] = 0;   cityY[5] = 0;   // another duplicate at origin
        cityX[6] = 1;   cityY[6] = 0;   // duplicate of city 2
        cityX[7] = 0;   cityY[7] = 1;   // duplicate of city 3
        cityX[8] = 1;   cityY[8] = 1;   // duplicate of city 4

        // Additional points at extremes of 0/1 to keep variety
        cityX[9] = 0;   cityY[9] = 0;
        cityX[10] = 1;  cityY[10] = 1;
        cityX[11] = 0;  cityY[11] = 1;
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
