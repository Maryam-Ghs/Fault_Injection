/* LLM input variant 6: ordered-structured */

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
    // Build a highly structured graph:
    //   - self‑loops on every vertex (diagonal = 1)
    //   - each vertex connected to its immediate neighbor (i+1 mod n)
    //   - symmetry ensures A is symmetric and circulant‑like
    // -----------------------------------------------------------------
    void buildEdgeHeavy() {
        // initialise to zero
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                A[i * n + j] = 0.0f;
            }
        }

        // self‑loops
        for (int i = 0; i < n; ++i) {
            A[i * n + i] = 1.0f;
        }

        // neighbor connections (circulant pattern)
        for (int i = 0; i < n; ++i) {
            int j = (i + 1) % n;
            A[i * n + j] = 1.0f;
            A[j * n + i] = 1.0f;  // ensure symmetry
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
    // Choose a size that triggers the unrolled sections (>=4)
    int size = 8;                     // structured circulant graph size
    AdjSpectrum solver(size);

    solver.buildEdgeHeavy();          // internal input generation
    solver.qrIterations(25);          // enough iterations for convergence
    solver.outputSpectrum();          // print results

    return 0;
}
