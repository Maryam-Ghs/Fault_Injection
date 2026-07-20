/* LLM input variant 3: zeros-and-ones */
// Version #6 – Adjacency Matrix Multiplication for Paths
// -----------------------------------------------------
// Implements matrix multiplication (A * B) on adjacency matrices.
// - Uses only int types.
// - All data lives on the stack.
// - Manual loop unrolling (factor 4) and fused arithmetic.
// - Class‑based design, no external input (generated internally).

#include <iostream>
#include <iomanip>

#define N 5   // Size of the adjacency matrices (edge‑case heavy when N is small)

class PathMul {
public:
    // Matrices are stored as plain 2‑D arrays on the stack.
    int matA[N][N];
    int matB[N][N];
    int matC[N][N];

    // Populate matrices with zero‑heavy, one‑heavy, and identity patterns.
    void generate() {
        // matA : identity matrix (ones on diagonal, zeros elsewhere)
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                matA[i][j] = (i == j) ? 1 : 0;
            }
        }

        // matB : mostly zeros, with a single chain of ones and a self‑loop at the last node
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                matB[i][j] = 0;
            }
        }
        matB[0][1] = 1;
        matB[1][2] = 1;
        matB[2][3] = 1;
        matB[3][4] = 1;
        matB[4][4] = 1;   // self‑loop at the last node
    }

    // Perform C = A * B using manual unrolling of the inner product.
    void multiply() {
        // Initialize result matrix to zero.
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j)
                matC[i][j] = 0;

        // Standard triple‑loop with unrolled k‑loop (step 4).
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                int acc = 0;
                int k = 0;
                // Unrolled segment (process four columns of A / rows of B each iteration)
                for (; k <= N - 4; k += 4) {
                    // Fused multiplication‑addition without temporaries
                    acc += matA[i][k]   * matB[k][j]
                         + matA[i][k+1] * matB[k+1][j]
                         + matA[i][k+2] * matB[k+2][j]
                         + matA[i][k+3] * matB[k+3][j];
                }
                // Remainder handling (if N is not a multiple of 4)
                for (; k < N; ++k) {
                    acc += matA[i][k] * matB[k][j];
                }
                matC[i][j] = acc;
            }
        }
    }

    // Pretty‑print a matrix with a title.
    void show(const char *title, int M[N][N]) {
        std::cout << title << ":\n";
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                std::cout << std::setw(3) << M[i][j] << ' ';
            }
            std::cout << '\n';
        }
        std::cout << '\n';
    }

    // Run the whole demonstration.
    void run() {
        generate();
        show("Matrix A (Adjacency)", matA);
        show("Matrix B (Adjacency)", matB);
        multiply();
        show("Result C = A * B", matC);
    }
};

int main() {
    PathMul demo;
    demo.run();
    return 0;
}
