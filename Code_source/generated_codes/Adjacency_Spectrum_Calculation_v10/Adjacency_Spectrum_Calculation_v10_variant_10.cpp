// LLM input variant 10: large-safe-stress
// Adjacency Spectrum Calculation – version #10
// Implements a simple power‑iteration based eigenvalue extractor.
// Uses only int and float, stack‑allocated arrays and a class‑based design.

#include <iostream>
#include <cmath>

class SpectrumCalc {
    static const int MAX_N = 50;    // increased maximum dimension
    int sz;                         // matrix dimension
    float A[MAX_N][MAX_N];           // adjacency matrix (max 50×50)
public:
    SpectrumCalc() : sz(20) {        // predefined 20‑node ring graph
        // Initialize all entries to zero
        for (int i = 0; i < MAX_N; ++i)
            for (int j = 0; j < MAX_N; ++j)
                A[i][j] = 0.0f;

        // Build an undirected ring: each node i connects to (i+1)%sz
        for (int i = 0; i < sz; ++i) {
            int j = (i + 1) % sz;
            A[i][j] = 1.0f;
            A[j][i] = 1.0f;
        }
    }

    void run() {
        int k = 0;
        while (k < sz) {
            float v[MAX_N];
            int i = 0;
            while (i < sz) v[i++] = 1.0f;               // start vector of ones

            int it = 0;
            while (it < 30) {                         // fixed iterations
                float w[MAX_N] = {0};
                int r = 0;
                while (r < sz) {
                    int c = 0;
                    while (c < sz) {
                        w[r] = w[r] + A[r][c] * v[c];
                        ++c;
                    }
                    ++r;
                }
                float nrm = 0.0f;
                i = 0;
                while (i < sz) nrm = nrm + w[i] * w[i], ++i;
                nrm = std::sqrt(nrm);
                i = 0;
                while (i < sz) v[i] = w[i] / nrm, ++i;
                ++it;
            }

            // Rayleigh quotient for eigenvalue
            float Av[MAX_N] = {0};
            i = 0;
            while (i < sz) {
                int j = 0;
                while (j < sz) {
                    Av[i] = Av[i] + A[i][j] * v[j];
                    ++j;
                }
                ++i;
            }
            float eig = 0.0f;
            i = 0;
            while (i < sz) eig = eig + v[i] * Av[i], ++i;

            std::cout << "Eigenvalue " << k + 1 << " : " << eig << '\n';

            // Deflation: A ← A – eig·v·vᵀ
            i = 0;
            while (i < sz) {
                int j = 0;
                while (j < sz) {
                    A[i][j] = A[i][j] - eig * v[i] * v[j];
                    ++j;
                }
                ++i;
            }
            ++k;
        }
    }
};

int main() {
    SpectrumCalc solver;
    solver.run();
    return 0;
}
