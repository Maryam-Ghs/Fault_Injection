/* LLM input variant 10: large-safe-stress */

// version #3 – Adjacency Spectrum Calculation (float only)

#include <iostream>
#include <cmath>   // only for sqrt, works with float

// ------------------------------------------------------------
// Class handling a graph's adjacency matrix and its spectrum
// ------------------------------------------------------------
class AdjSpectrum {
    int n;          // matrix dimension
    float* A;       // adjacency / current matrix (heap allocated)

public:
    AdjSpectrum(int dim) {
        n = dim;
        A = new float[n * n];
    }

    ~AdjSpectrum() {
        delete[] A;
    }

    // -----------------------------------------------------------------
    // Build a graph that stresses edge‑cases:
    //   - self‑loops on every vertex
    //   - first half forms a complete subgraph
    //   - second half are isolated (except self‑loops)
    //   - a single extra edge connecting the halves
    // -----------------------------------------------------------------
    void buildEdgeHeavy() {
        int i, j;
        for (i = 0; i < n; ++i) {
            for (j = 0; j < n; ++j) {
                A[i * n + j] = 0.0f;
            }
        }

        // self‑loops
        for (i = 0; i < n; ++i) {
            A[i * n + i] = 1.0f;
        }

        // complete subgraph on first half
        int half = n / 2;
        for (i = 0; i < half; ++i) {
            for (j = i + 1; j < half; ++j) {
                A[i * n + j] = 1.0f;
                A[j * n + i] = 1.0f;
            }
        }

        // a single bridging edge (if possible)
        if (half > 0 && half < n) {
            A[0 * n + half] = 1.0f;
            A[half * n + 0] = 1.0f;
        }
    }

    // -------------------------------------------------------------
    // Perform a fixed number of QR iterations (A = R * Q)
    // using Gram‑Schmidt with manual loop unrolling.
    // -------------------------------------------------------------
    void qrIterations(int steps) {
        int step, i, j, k;

        // temporary storage for Q and R
        float* Q = new float[n * n];
        float* R = new float[n * n];

        for (step = 0; step < steps; ++step) {
            // ---- Gram‑Schmidt: compute Q and R ----
            for (j = 0; j < n; ++j) {
                // copy column j of A into temporary vector v
                float v0 = A[0 * n + j];
                float v1 = A[1 * n + j];
                float v2 = A[2 * n + j];
                float v3 = A[3 * n + j];
                // (for larger n we continue similarly)
                // compute projections onto previous q's
                for (k = 0; k < j; ++k) {
                    // dot product q_k • v (unrolled for first four rows)
                    float dot = Q[0 * n + k] * v0 +
                                Q[1 * n + k] * v1 +
                                Q[2 * n + k] * v2 +
                                Q[3 * n + k] * v3;
                    // subtract projection
                    v0 -= dot * Q[0 * n + k];
                    v1 -= dot * Q[1 * n + k];
                    v2 -= dot * Q[2 * n + k];
                    v3 -= dot * Q[3 * n + k];
                    // store R entry
                    R[k * n + j] = dot;
                }
                // norm of the residual vector (unrolled)
                float norm = std::sqrt(v0 * v0 + v1 * v1 + v2 * v2 + v3 * v3);
                // avoid division by zero
                if (norm < 1e-6f) norm = 1e-6f;
                // normalize to obtain q_j (unrolled)
                Q[0 * n + j] = v0 / norm;
                Q[1 * n + j] = v1 / norm;
                Q[2 * n + j] = v2 / norm;
                Q[3 * n + j] = v3 / norm;
                // store diagonal of R
                R[j * n + j] = norm;
                // zero the rest of column j in R
                for (i = j + 1; i < n; ++i) {
                    R[i * n + j] = 0.0f;
                }
            }

            // ---- Form A = R * Q (unrolled for first four rows) ----
            for (i = 0; i < n; ++i) {
                for (j = 0; j < n; ++j) {
                    // compute row i of R • column j of Q
                    float sum0 = R[i * n + 0] * Q[0 * n + j];
                    float sum1 = R[i * n + 1] * Q[1 * n + j];
                    float sum2 = R[i * n + 2] * Q[2 * n + j];
                    float sum3 = R[i * n + 3] * Q[3 * n + j];
                    // accumulate (more rows could be added similarly)
                    A[i * n + j] = sum0 + sum1 + sum2 + sum3;
                }
            }
        }

        delete[] Q;
        delete[] R;
    }

    // -------------------------------------------------------------
    // Print the approximated eigenvalues (diagonal of A)
    // -------------------------------------------------------------
    void outputSpectrum() {
        int i;
        std::cout << "Approximated eigenvalues (float):\n";
        for (i = 0; i < n; ++i) {
            std::cout << "  λ[" << i << "] = " << A[i * n + i] << "\n";
        }
    }
};

// -------------------------------------------------------------
// Entry point – generate a challenging graph and compute its spectrum
// -------------------------------------------------------------
int main() {
    // Choose a size that triggers the unrolled sections (>=4) and provides a larger stress test
    int size = 200;                     // larger safe‑stress size
    AdjSpectrum solver(size);

    solver.buildEdgeHeavy();          // internal input generation
    solver.qrIterations(25);          // enough iterations for convergence
    solver.outputSpectrum();          // print results

    return 0;
}
