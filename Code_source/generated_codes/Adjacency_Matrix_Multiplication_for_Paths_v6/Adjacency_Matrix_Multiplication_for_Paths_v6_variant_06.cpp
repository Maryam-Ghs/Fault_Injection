/* LLM input variant 6: ordered-structured */
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

    // Populate matrices with highly structured, ordered data.
    void generate() {
        // matA : symmetric matrix with increasing values.
        // Value at (i,j) = i + j + 1, which yields a sorted, monotonic pattern.
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j <= i; ++j) {
                int val = i + j + 1;
                matA[i][j] = val;
                matA[j][i] = val;   // enforce symmetry
            }
        }

        // matB : each row is a nearly sorted sequence.
        // Row i contains values i, i+1, ..., i+N-1 modulo N.
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                matB[i][j] = (i + j) % N;
            }
        }
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
        show("Matrix A (Ordered Symmetric)", matA);
        show("Matrix B (Ordered Nearly Sorted)", matB);
        multiply();
        show("Result C = A * B", matC);
    }
};

int main() {
    PathMul demo;
    demo.run();
    return 0;
}
