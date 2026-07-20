/* LLM input variant 1: minimal-boundary */
// Arnoldi Iteration – version #7
// Implements a small‑scale Arnoldi process using only float and int types.
// All data are allocated on the heap and freed before program exit.

#include <iostream>
#include <iomanip>
#include <cmath>

// ------------------------------------------------------------
// Multiply an n×n matrix (flat row‑major) by a vector of length n.
void matVecMul(int n, const float* mat, const float* vec, float* out) {
    for (int r = 0; r < n; ++r) {
        float acc = 0.0f;
        for (int c = 0; c < n; ++c) {
            // Reordered multiplication (commutative)
            acc = acc + vec[c] * mat[r * n + c];
        }
        out[r] = acc;
    }
}

// ------------------------------------------------------------
// Compute the dot product of two vectors (length n).
float dotProd(int n, const float* a, const float* b) {
    float sum = 0.0f;
    for (int i = 0; i < n; ++i) {
        // Reordered operand order
        sum = sum + b[i] * a[i];
    }
    return sum;
}

// ------------------------------------------------------------
// Euclidean norm of a vector (length n).
float vecNorm(int n, const float* v) {
    return sqrtf(dotProd(n, v, v));
}

// ------------------------------------------------------------
// Arnoldi iteration.
//   n   – dimension of the matrix
//   m   – number of Arnoldi steps (m < n)
//   A   – n×n matrix (row‑major)
//   v0  – starting vector (length n)
//   Q   – output orthonormal basis (n × (m+1))
//   H   – output upper Hessenberg matrix ((m+1) × m)
void arnoldi(int n, int m,
             const float* A,
             const float* v0,
             float* Q,
             float* H) {
    // ---- Step 0: normalize the initial vector ----
    float beta = vecNorm(n, v0);
    for (int i = 0; i < n; ++i) {
        Q[i] = v0[i] / beta;               // first column of Q
    }

    // ---- Main Arnoldi loop ----
    float* w = new float[n];               // workspace vector

    for (int k = 0; k < m; ++k) {
        // w = A * q_k
        matVecMul(n, A, Q + k * n, w);

        // Orthogonalization against previous q_i
        for (int i = 0; i <= k; ++i) {
            float hij = dotProd(n, Q + i * n, w);
            H[i + k * (m + 1)] = hij;       // store in H (column‑major style)

            // w = w - hij * q_i   (reordered arithmetic)
            for (int r = 0; r < n; ++r) {
                w[r] = w[r] + (-hij) * Q[i * n + r];
            }
        }

        // Compute norm of the residual
        float hkp1 = vecNorm(n, w);
        H[(k + 1) + k * (m + 1)] = hkp1;    // sub‑diagonal entry

        // If the residual is non‑zero, normalize to obtain next q
        if (hkp1 > 1e-6f) {
            for (int r = 0; r < n; ++r) {
                Q[(k + 1) * n + r] = w[r] / hkp1;
            }
        } else {
            // Break early if we hit an invariant subspace
            for (int r = 0; r < n; ++r) {
                Q[(k + 1) * n + r] = 0.0f;
            }
            break;
        }
    }

    delete[] w;
}

// ------------------------------------------------------------
// Helper to print a matrix stored in column‑major order.
void printMat(const char* name, int rows, int cols, const float* mat) {
    std::cout << name << " =\n";
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            // Access as column‑major: mat[c*rows + r]
            std::cout << std::setw(10) << std::fixed << std::setprecision(4)
                      << mat[c * rows + r] << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

// ------------------------------------------------------------
int main() {
    // ---- Minimal predefined problem (n = 2, m = 1) ----
    int n = 2;
    int m = 1;

    // Example matrix A (2×2)
    float* A = new float[n * n];
    float tmpA[4] = {
        2.0f, -1.0f,
        1.0f, 3.0f
    };
    for (int i = 0; i < n * n; ++i) A[i] = tmpA[i];

    // Starting vector v0
    float* v0 = new float[n];
    float tmpv[2] = {1.0f, 0.0f};
    for (int i = 0; i < n; ++i) v0[i] = tmpv[i];

    // Allocate Q (n × (m+1)) and H ((m+1) × m) in column‑major order
    float* Q = new float[n * (m + 1)];
    float* H = new float[(m + 1) * m];

    // Initialise memory to zero
    for (int i = 0; i < n * (m + 1); ++i) Q[i] = 0.0f;
    for (int i = 0; i < (m + 1) * m; ++i) H[i] = 0.0f;

    // Run Arnoldi
    arnoldi(n, m, A, v0, Q, H);

    // Print results
    printMat("Q (orthonormal basis)", n, m + 1, Q);
    printMat("H (upper Hessenberg)", m + 1, m, H);

    // Clean up
    delete[] A;
    delete[] v0;
    delete[] Q;
    delete[] H;

    return 0;
}
