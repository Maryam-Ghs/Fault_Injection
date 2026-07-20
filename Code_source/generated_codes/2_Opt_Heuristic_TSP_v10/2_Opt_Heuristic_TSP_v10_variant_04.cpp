// 2‑Opt Heuristic for TSP – version 10
// ------------------------------------------------------------
//  * only int and float are used
//  * all data are allocated on the heap (new / delete)
//  * deterministic city coordinates are generated inside the program
//  * results (tour and length) are printed to stdout
// ------------------------------------------------------------
/* LLM input variant 4: signed-extremes */

#include <iostream>
#include <cmath>
#include <iomanip>

class TspTwoOpt {
public:
    // number of cities
    int  cnt;

    // coordinates
    float*  posX;
    float*  posY;

    // distance matrix
    float**  dmat;

    // current tour (last element repeats the start city)
    int*  tour;

    // --------------------------------------------------------
    TspTwoOpt(int n) : cnt(n) {
        // allocate coordinate arrays
        posX = new float[cnt];
        posY = new float[cnt];

        // allocate distance matrix
        dmat = new float*[cnt];
        for (int i = 0; i < cnt; ++i) {
            dmat[i] = new float[cnt];
        }

        // allocate tour array (cnt+1 because of return to start)
        tour = new int[cnt + 1];

        // generate deterministic test data
        generate_points();

        // build distance matrix
        build_matrix();

        // initialise a simple sequential tour
        init_tour();
    }

    // --------------------------------------------------------
    ~TspTwoOpt() {
        // free distance matrix rows
        for (int i = 0; i < cnt; ++i) {
            delete[] dmat[i];
        }
        delete[] dmat;

        // free other heap allocations
        delete[] posX;
        delete[] posY;
        delete[] tour;
    }

    // --------------------------------------------------------
    void solve() {
        bool   improved = true;

        // repeat until no improving 2‑opt move exists
        while (improved) {
            improved = false;

            // try every possible pair (i , j) with i < j‑1
            int i = 1;
            while (i < cnt - 2) {
                int j = i + 2;
                while (j < cnt) {
                    // current edges: (i‑1,i) and (j,j+1)
                    int a = tour[i - 1];
                    int b = tour[i];
                    int c = tour[j];
                    int d = tour[j + 1];

                    // compute gain = oldDist - newDist
                    float oldSeg = dmat[a][b] + dmat[c][d];
                    float newSeg = dmat[a][c] + dmat[b][d];
                    float gain   = oldSeg - newSeg;

                    // if improvement, apply 2‑opt swap
                    if (gain > 0.0f) {
                        // reverse the segment between i and j
                        int left  = i;
                        int right = j;
                        while (left < right) {
                            int tmp      = tour[left];
                            tour[left]   = tour[right];
                            tour[right]  = tmp;
                            ++left;
                            --right;
                        }
                        improved = true;
                    }
                    ++j;
                }
                ++i;
            }
        }
    }

    // --------------------------------------------------------
    float total_length() const {
        float sum = 0.0f;
        for (int k = 0; k < cnt; ++k) {
            int from = tour[k];
            int to   = tour[k + 1];
            sum += dmat[from][to];
        }
        return sum;
    }

    // --------------------------------------------------------
    void print_result() const {
        std::cout << "Tour length : " << std::fixed << std::setprecision(3)
                  << total_length() << "\n";
        std::cout << "Tour order  : ";
        for (int k = 0; k <= cnt; ++k) {
            std::cout << tour[k];
            if (k < cnt) std::cout << " -> ";
        }
        std::cout << "\n";
    }

private:
    // --------------------------------------------------------
    void generate_points() {
        // deterministic coordinates for 8 cities with mixed signs
        float presetX[8] = { -10.0f, -5.0f, 0.0f, 5.0f,
                             10.0f, -7.0f, 3.0f, -2.0f };
        float presetY[8] = { 15.0f, -15.0f, 0.0f, 8.0f,
                             -8.0f, 12.0f, -12.0f, 0.0f };

        for (int i = 0; i < cnt; ++i) {
            posX[i] = presetX[i];
            posY[i] = presetY[i];
        }
    }

    // --------------------------------------------------------
    void build_matrix() {
        for (int p = 0; p < cnt; ++p) {
            for (int q = 0; q < cnt; ++q) {
                // Euclidean distance (float arithmetic)
                float dx = posX[p] - posX[q];
                float dy = posY[p] - posY[q];
                float tmp = dx * dx + dy * dy;
                dmat[p][q] = sqrtf(tmp);
            }
        }
    }

    // --------------------------------------------------------
    void init_tour() {
        for (int i = 0; i < cnt; ++i) {
            tour[i] = i;
        }
        // close the loop
        tour[cnt] = tour[0];
    }
};

// ------------------------------------------------------------
int main() {
    // create a solver for 8 cities
    TspTwoOpt solver(8);

    // run the 2‑Opt heuristic
    solver.solve();

    // output the final tour and its length
    solver.print_result();

    return 0;
}
