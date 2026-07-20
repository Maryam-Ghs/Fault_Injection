#include <iostream>
#include <cstdlib>
#include <ctime>
#include <iomanip>

#define ROWS 200      // number of users (larger safe stress)
#define COLS 300      // number of items (larger safe stress)
#define FACT 5        // latent factors
#define ITER 10       // ALS iterations
#define LAMB 0.1f     // regularization

/* LLM input variant 10: large-safe-stress */

int main() {
    // deterministic seed for reproducible test data
    std::srand(0);

    // ----- generate rating matrix (ROW x COL) -----
    float R[ROWS][COLS];
    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c < COLS; ++c) {
            // deterministic pseudo‑random rating 0..5, zero means missing
            R[r][c] = static_cast<float>(std::rand() % 6);
        }
    }

    // ----- factor matrices: USER (ROW x FACT) & ITEM (COL x FACT) -----
    float U[ROWS][FACT];
    float V[COLS][FACT];
    for (int i = 0; i < ROWS; ++i)
        for (int f = 0; f < FACT; ++f)
            U[i][f] = 0.1f * static_cast<float>(std::rand() % 10 + 1);
    for (int j = 0; j < COLS; ++j)
        for (int f = 0; f < FACT; ++f)
            V[j][f] = 0.1f * static_cast<float>(std::rand() % 10 + 1);

    // ----- ALS core -----
    for (int it = 0; it < ITER; ++it) {
        // ---- update USER factors (U) ----
        for (int i = 0; i < ROWS; ++i) {
            // compute A = V^T * V + LAMB * I   (FACT x FACT)
            float A[FACT][FACT] = { {LAMB, 0.0f, 0.0f, 0.0f, 0.0f},
                                    {0.0f, LAMB, 0.0f, 0.0f, 0.0f},
                                    {0.0f, 0.0f, LAMB, 0.0f, 0.0f},
                                    {0.0f, 0.0f, 0.0f, LAMB, 0.0f},
                                    {0.0f, 0.0f, 0.0f, 0.0f, LAMB} };
            float b[FACT] = {0.0f};

            for (int j = 0; j < COLS; ++j) {
                float rij = R[i][j];
                for (int p = 0; p < FACT; ++p) {
                    float vp = V[j][p];
                    b[p] += vp * rij;
                    for (int q = 0; q < FACT; ++q) {
                        A[p][q] += vp * V[j][q];
                    }
                }
            }

            // solve small linear system A * x = b using Gaussian elimination (FACT <= 5)
            // Simple Gaussian elimination without pivoting (safe for this size)
            float x[FACT];
            // copy A and b to mutable locals
            float M[FACT][FACT];
            float y[FACT];
            for (int p = 0; p < FACT; ++p) {
                y[p] = b[p];
                for (int q = 0; q < FACT; ++q) {
                    M[p][q] = A[p][q];
                }
            }

            // forward elimination
            for (int p = 0; p < FACT; ++p) {
                // avoid zero pivot
                if (M[p][p] == 0.0f) M[p][p] = 1e-6f;
                float piv = M[p][p];
                for (int q = p; q < FACT; ++q) M[p][q] /= piv;
                y[p] /= piv;
                for (int r = p + 1; r < FACT; ++r) {
                    float factor = M[r][p];
                    for (int q = p; q < FACT; ++q) M[r][q] -= factor * M[p][q];
                    y[r] -= factor * y[p];
                }
            }
            // back substitution
            for (int p = FACT - 1; p >= 0; --p) {
                float sum = 0.0f;
                for (int q = p + 1; q < FACT; ++q) sum += M[p][q] * x[q];
                x[p] = y[p] - sum;
            }

            for (int f = 0; f < FACT; ++f) {
                U[i][f] = x[f];
            }
        }

        // ---- update ITEM factors (V) ----
        for (int j = 0; j < COLS; ++j) {
            // compute A = U^T * U + LAMB * I   (FACT x FACT)
            float A[FACT][FACT] = { {LAMB, 0.0f, 0.0f, 0.0f, 0.0f},
                                    {0.0f, LAMB, 0.0f, 0.0f, 0.0f},
                                    {0.0f, 0.0f, LAMB, 0.0f, 0.0f},
                                    {0.0f, 0.0f, 0.0f, LAMB, 0.0f},
                                    {0.0f, 0.0f, 0.0f, 0.0f, LAMB} };
            float b[FACT] = {0.0f};

            for (int i = 0; i < ROWS; ++i) {
                float rij = R[i][j];
                for (int p = 0; p < FACT; ++p) {
                    float up = U[i][p];
                    b[p] += up * rij;
                    for (int q = 0; q < FACT; ++q) {
                        A[p][q] += up * U[i][q];
                    }
                }
            }

            // solve A * x = b (same Gaussian elimination)
            float x[FACT];
            float M[FACT][FACT];
            float y[FACT];
            for (int p = 0; p < FACT; ++p) {
                y[p] = b[p];
                for (int q = 0; q < FACT; ++q) {
                    M[p][q] = A[p][q];
                }
            }

            for (int p = 0; p < FACT; ++p) {
                if (M[p][p] == 0.0f) M[p][p] = 1e-6f;
                float piv = M[p][p];
                for (int q = p; q < FACT; ++q) M[p][q] /= piv;
                y[p] /= piv;
                for (int r = p + 1; r < FACT; ++r) {
                    float factor = M[r][p];
                    for (int q = p; q < FACT; ++q) M[r][q] -= factor * M[p][q];
                    y[r] -= factor * y[p];
                }
            }
            for (int p = FACT - 1; p >= 0; --p) {
                float sum = 0.0f;
                for (int q = p + 1; q < FACT; ++q) sum += M[p][q] * x[q];
                x[p] = y[p] - sum;
            }

            for (int f = 0; f < FACT; ++f) {
                V[j][f] = x[f];
            }
        }
    }

    // ----- output result matrices -----
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "User factors (U):\n";
    for (int i = 0; i < ROWS; ++i) {
        for (int f = 0; f < FACT; ++f)
            std::cout << U[i][f] << ' ';
        std::cout << '\n';
    }
    std::cout << "\nItem factors (V):\n";
    for (int j = 0; j < COLS; ++j) {
        for (int f = 0; f < FACT; ++f)
            std::cout << V[j][f] << ' ';
        std::cout << '\n';
    }

    // optional: reconstruct approximation and show error
    std::cout << "\nReconstructed ratings (U*V^T) [first 10 rows and cols]:\n";
    for (int i = 0; i < std::min(10, ROWS); ++i) {
        for (int j = 0; j < std::min(10, COLS); ++j) {
            float est = 0.0f;
            for (int f = 0; f < FACT; ++f)
                est += U[i][f] * V[j][f];
            std::cout << est << ' ';
        }
        std::cout << '\n';
    }
    return 0;
}
