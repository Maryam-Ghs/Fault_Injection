/* LLM input variant 10: large-safe-stress */
// Arnoldi Iteration – version #7 (large safe stress input)

#include <iostream>
#include <iomanip>
#include <cmath>

void matVecMul(int n, const float* mat, const float* vec, float* out) {
    for (int r = 0; r < n; ++r) {
        float acc = 0.0f;
        for (int c = 0; c < n; ++c) {
            acc = acc + vec[c] * mat[r * n + c];
        }
        out[r] = acc;
    }
}

float dotProd(int n, const float* a, const float* b) {
    float sum = 0.0f;
    for (int i = 0; i < n; ++i) {
        sum = sum + b[i] * a[i];
    }
    return sum;
}

float vecNorm(int n, const float* v) {
    return sqrtf(dotProd(n, v, v));
}

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

int main() {
    // Large but safe stress test (n = 200, m = 150)
    int n = 200;
    int m = 150;

    // Tridiagonal matrix (2 on diagonal, -1 on sub‑ and super‑diagonal)
    float* A = new float[n * n];
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (i == j) {
                A[i * n + j] = 2.0f;
            } else if (std::abs(i - j) == 1) {
                A[i * n + j] = -1.0f;
            } else {
                A[i * n + j] = 0.0f;
            }
        }
    }

    // Starting vector of all ones
    float* v0 = new float[n];
    for (int i = 0; i < n; ++i) v0[i] = 1.0f;

    float* Q = new float[n * (m + 1)];
    float* H = new float[(m + 1) * m];

    for (int i = 0; i < n * (m + 1); ++i) Q[i] = 0.0f;
    for (int i = 0; i < (m + 1) * m; ++i) H[i] = 0.0f;

    arnoldi(n, m, A, v0, Q, H);

    // Print only a small excerpt to keep output manageable
    const int previewRows = 5;
    const int previewColsQ = std::min(m + 1, 5);
    const int previewColsH = std::min(m, 5);

    std::cout << "Q (first " << previewRows << " rows, " << previewColsQ << " cols) =\n";
    for (int r = 0; r < previewRows; ++r) {
        for (int c = 0; c < previewColsQ; ++c) {
            std::cout << std::setw(10) << std::fixed << std::setprecision(4)
                      << Q[c * n + r] << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";

    std::cout << "H (first " << previewRows << " rows, " << previewColsH << " cols) =\n";
    for (int r = 0; r < previewRows; ++r) {
        for (int c = 0; c < previewColsH; ++c) {
            std::cout << std::setw(10) << std::fixed << std::setprecision(4)
                      << H[c * (m + 1) + r] << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";

    delete[] A;
    delete[] v0;
    delete[] Q;
    delete[] H;

    return 0;
}
