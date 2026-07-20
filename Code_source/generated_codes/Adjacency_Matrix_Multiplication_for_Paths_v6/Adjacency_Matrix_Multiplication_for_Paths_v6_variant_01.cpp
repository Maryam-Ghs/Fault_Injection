/* LLM input variant 1: minimal-boundary */
// Version #6 – Adjacency Matrix Multiplication for Paths
// -----------------------------------------------------
// Implements matrix multiplication (A * B) on adjacency matrices.
// - Uses only int types.
// - All data lives on the stack.
// - Manual loop unrolling (factor 4) and fused arithmetic.
// - Class‑based design, no external input (generated internally).

#include <iostream>
#include <iomanip>

#define N 2   // Minimal non‑trivial size for adjacency matrices

class PathMul {
public:
    // Matrices are stored as plain 2‑D arrays on the stack.
    int matA[N][N];
    int matB[N][N];
    int matC[N][N];

    // Populate matrices with boundary‑valid patterns.
    void generate() {
        // matA : node 0 has a self‑loop, node 1 is isolated.
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                if (i == 1) {                 // isolated row/col
                    matA[i][j] = 0;
                } else if (i == j && i == 0) { // self‑loop on node 0
                    matA[i][j] = 1;
                } else {
                    matA[i][j] = 1;               // dense connections otherwise
                }
            }
        }

        // matB : single edge from node 0 to 1 and a self‑loop on node 1.
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                matB[i][j] = 0;
            }
        }
        matB[0][1] = 1;   // edge 0 -> 1
        matB[1][1] = 1;   // self‑loop at node 1
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
