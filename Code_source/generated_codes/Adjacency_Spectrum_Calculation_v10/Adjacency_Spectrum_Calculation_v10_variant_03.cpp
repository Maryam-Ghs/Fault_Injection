/* LLM input variant 3: zeros-and-ones */
// Adjacency Spectrum Calculation – version #10
// Implements a simple power‑iteration based eigenvalue extractor.
// Uses only int and float, stack‑allocated arrays and a class‑based design.

#include <iostream>
#include <cmath>

class SpectrumCalc {
    int sz;                         // matrix dimension
    float A[5][5];                  // adjacency matrix (max 5×5)
public:
    SpectrumCalc() : sz(5) {        // predefined 5‑node graph (identity matrix)
        for (int i = 0; i < sz; ++i) {
            for (int j = 0; j < sz; ++j) {
                A[i][j] = (i == j) ? 1.0f : 0.0f;  // zeros and ones only
            }
        }
    }

    void run() {
        int k=0;
        while (k<sz) {
            float v[5];
            int i=0;
            while (i<sz) v[i++]=1.0f;               // start vector of ones

            int it=0;
            while (it<30) {                         // fixed iterations
                float w[5]={0};
                int r=0;
                while (r<sz) {
                    int c=0;
                    while (c<sz) {
                        w[r] = w[r] + A[r][c] * v[c];
                        ++c;
                    }
                    ++r;
                }
                float nrm=0.0f;
                i=0;
                while (i<sz) nrm = nrm + w[i]*w[i], ++i;
                nrm = std::sqrt(nrm);
                i=0;
                while (i<sz) v[i] = w[i] / nrm, ++i;
                ++it;
            }

            // Rayleigh quotient for eigenvalue
            float Av[5]={0};
            i=0;
            while (i<sz) {
                int j=0;
                while (j<sz) {
                    Av[i] = Av[i] + A[i][j] * v[j];
                    ++j;
                }
                ++i;
            }
            float eig=0.0f;
            i=0;
            while (i<sz) eig = eig + v[i]*Av[i], ++i;

            std::cout << "Eigenvalue " << k+1 << " : " << eig << '\n';

            // Deflation: A ← A – eig·v·vᵀ
            i=0;
            while (i<sz) {
                int j=0;
                while (j<sz) {
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
