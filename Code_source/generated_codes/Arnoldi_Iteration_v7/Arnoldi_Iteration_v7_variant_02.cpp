/* LLM input variant 2: small-diverse */
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
void arnoldi(int n, int m,
             const float* A,
             const float* v0,
             float* Q,
             float* H) {
    float beta = vecNorm(n, v0);
    for (int i = 0; i < n; ++i) {
        Q[i] = v0[i] / beta;
    }

    float* w = new float[n];

    for (int k = 0; k < m; ++k) {
        matVecMul(n, A, Q + k * n, w);
        for (int i = 0; i <= k; ++i) {
            float hij = dotProd(n, Q + i * n, w);
            H[i + k * (m + 1)] = hij;
            for (int r = 0; r < n; ++r) {
                w[r] = w[r] + (-hij) * Q[i * n + r];
            }
        }
        float hkp1 = vecNorm(n, w);
        H[(k + 1) + k * (m + 1)] = hkp1;
        if (hkp1 > 1e-6f) {
            for (int r = 0; r < n; ++r) {
                Q[(k + 1) * n + r] = w[r] / hkp1;
            }
        } else {
            for (int r = 0; r < n; ++r) {
                Q[(k + 1) * n + r] = 0.0f;
            }
            break;
        }
    }

    delete[] w;
}

// ------------------------------------------------------------
void printMat(const char* name, int rows, int cols, const float* mat) {
    std::cout << name << " =\n";
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            std::cout << std::setw(10) << std::fixed << std::setprecision(4)
                      << mat[c * rows + r] << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

// ------------------------------------------------------------
int main() {
    // ---- Small diverse problem (n = 5, m = 3) ----
    int n = 5;
    int m = 3;

    // Example matrix A (5×5) with varied values
    float* A = new float[n * n];
    float tmpA[25] = {
        5.0f, -2.0f, 0.0f,  1.0f,  3.0f,
        0.5f,  4.0f, -1.5f, 0.0f,  2.0f,
        1.0f,  0.0f,  3.5f, -2.0f, 0.0f,
        -1.0f, 2.0f,  0.0f,  4.0f, 1.5f,
        0.0f,  1.0f,  2.0f, -3.0f, 5.0f
    };
    for (int i = 0; i < n * n; ++i) A[i] = tmpA[i];

    // Starting vector v0 with varied entries
    float* v0 = new float[n];
    float tmpv[5] = {0.7f, 1.2f, -0.5f, 0.0f, 2.3f};
    for (int i = 0; i < n; ++i) v0[i] = tmpv[i];

    // Allocate Q and H
    float* Q = new float[n * (m + 1)];
    float* H = new float[(m + 1) * m];

    // Initialise to zero
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
