#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <ctime>
#include <cmath>

#define MAXN 30          // maximum matrix dimension (stack allocation)
#define STEPS 10         // number of Arnoldi steps (<= MAXN)

/* LLM input variant 10: large-safe-stress */

class ArnoldiSolver {
public:
    int dim;                     // matrix size
    int steps;                   // Krylov subspace dimension
    float A[MAXN][MAXN];         // input matrix
    float Q[MAXN][MAXN];         // orthonormal basis
    float H[MAXN][MAXN];         // upper Hessenberg matrix

    // -----------------------------------------------------------------
    // Fill the matrix with a deterministic dense pattern that stresses size
    // -----------------------------------------------------------------
    void randomize_matrix() {
        for (int r = 0; r < dim; ++r) {
            for (int c = 0; c < dim; ++c) {
                // Example pattern: (i * j + 1) modulo 100, scaled to [0,1)
                int val = (r * c + 1) % 100;
                A[r][c] = static_cast<float>(val) / 100.0f;
            }
        }
    }

    // -----------------------------------------------------------------
    // Compute Euclidean norm of a vector (size = dim)
    // -----------------------------------------------------------------
    float vec_norm(const float *vec) {
        float sum = 0.0f;
        for (int i = 0; i < dim; ++i) {
            float tmp = vec[i];
            sum += tmp * tmp;
        }
        return std::sqrt(sum);
    }

    // -----------------------------------------------------------------
    // Dot product of two vectors (size = dim)
    // -----------------------------------------------------------------
    float dot_product(const float *u, const float *v) {
        float acc = 0.0f;
        for (int i = 0; i < dim; ++i) {
            acc += u[i] * v[i];
        }
        return acc;
    }

    // -----------------------------------------------------------------
    // Multiply matrix A by vector src, store in dst (both size = dim)
    // -----------------------------------------------------------------
    void mat_vec_mul(const float src[MAXN], float dst[MAXN]) {
        for (int i = 0; i < dim; ++i) {
            float accum = 0.0f;
            for (int j = 0; j < dim; ++j) {
                accum += A[i][j] * src[j];
            }
            dst[i] = accum;
        }
    }

    // -----------------------------------------------------------------
    // Perform Arnoldi iteration
    // -----------------------------------------------------------------
    void run() {
        // ---- step 0 : deterministic dense start vector ----
        float start[MAXN];
        for (int i = 0; i < dim; ++i) {
            // Pattern: (i * 7 + 3) modulo 100, scaled to [0,1)
            int val = (i * 7 + 3) % 100;
            start[i] = static_cast<float>(val) / 100.0f;
        }
        float nrm0 = vec_norm(start);
        for (int i = 0; i < dim; ++i) {
            Q[i][0] = start[i] / nrm0;
        }

        // ---- main Arnoldi loop -----------------------------------------
        int k = 0;
        while (k < steps) {
            // w = A * q_k
            float w[MAXN];
            float qk[MAXN];
            for (int i = 0; i < dim; ++i) qk[i] = Q[i][k];
            mat_vec_mul(qk, w);

            // h_ik = q_i^T * w   for i = 0..k
            for (int i = 0; i <= k; ++i) {
                float qi[MAXN];
                for (int r = 0; r < dim; ++r) qi[r] = Q[r][i];
                float hij = dot_product(qi, w);
                H[i][k] = hij;
            }

            // w = w - sum_{i=0}^{k} h_ik * q_i
            for (int i = 0; i <= k; ++i) {
                float coeff = H[i][k];
                for (int r = 0; r < dim; ++r) {
                    w[r] -= coeff * Q[r][i];
                }
            }

            // h_{k+1,k} = ||w||
            float wk_norm = vec_norm(w);
            H[k + 1][k] = wk_norm;

            // if norm is non‑zero, create next basis vector
            if (wk_norm > 1e-6f && k + 1 < steps) {
                for (int i = 0; i < dim; ++i) {
                    Q[i][k + 1] = w[i] / wk_norm;
                }
            }

            ++k;
        }
    }

    // -----------------------------------------------------------------
    // Print matrix (size rows x cols)
    // -----------------------------------------------------------------
    void print_mat(const float mat[MAXN][MAXN], int rows, int cols, const char *name) {
        std::cout << name << " =\n";
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                std::cout << std::setw(10) << std::fixed << std::setprecision(4) << mat[r][c];
            }
            std::cout << '\n';
        }
        std::cout << '\n';
    }
};

int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    ArnoldiSolver solver;
    solver.dim   = 30;            // maximum allowed dimension to stress the algorithm
    solver.steps = 29;            // large number of Arnoldi steps, still safe (MAXN‑1)

    solver.randomize_matrix();    // deterministic large dense matrix A
    solver.run();                 // execute Arnoldi iteration

    // Print the first few basis vectors and the Hessenberg matrix
    solver.print_mat(solver.Q, solver.dim, solver.steps, "Q (orthonormal basis)");
    solver.print_mat(solver.H, solver.steps + 1, solver.steps, "H (Hessenberg)");

    return 0;
}
