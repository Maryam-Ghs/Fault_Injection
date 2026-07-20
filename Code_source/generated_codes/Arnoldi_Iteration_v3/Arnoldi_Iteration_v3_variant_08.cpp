#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <ctime>
#include <cmath>

#define MAXN 30          // maximum matrix dimension (stack allocation)
#define STEPS 10         // number of Arnoldi steps (<= MAXN)

/* LLM input variant 8: sparse-skewed */

class ArnoldiSolver {
public:
    int dim;                     // matrix size
    int steps;                   // Krylov subspace dimension
    float A[MAXN][MAXN];         // input matrix
    float Q[MAXN][MAXN];         // orthonormal basis
    float H[MAXN][MAXN];         // upper Hessenberg matrix

    // -----------------------------------------------------------------
    // Fill the matrix with a sparse, skewed pattern
    // -----------------------------------------------------------------
    void randomize_matrix() {
        // Start with all zeros
        for (int r = 0; r < dim; ++r) {
            for (int c = 0; c < dim; ++c) {
                A[r][c] = 0.0f;
            }
        }

        // Diagonal entries (identity like)
        for (int i = 0; i < dim; ++i) {
            A[i][i] = 1.0f;
        }

        // Small dense cluster in the top‑left 5x5 block
        int clusterSize = (dim < 5) ? dim : 5;
        for (int r = 0; r < clusterSize; ++r) {
            for (int c = 0; c < clusterSize; ++c) {
                // Give a modest value that decays with distance from diagonal
                A[r][c] = 0.2f * (1.0f - std::abs(r - c) * 0.1f);
            }
        }

        // One skewed large entry far from the diagonal
        if (dim > 1) {
            A[0][dim - 1] = 5.0f;   // heavily weighted off‑diagonal element
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
        // ---- step 0 : deterministic sparse start vector and normalize it ----
        float start[MAXN];
        for (int i = 0; i < dim; ++i) {
            start[i] = (i == 0) ? 1.0f : 0.0f;   // only first component non‑zero
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
    solver.dim   = 20;            // choose a size <= MAXN
    solver.steps = STEPS;         // number of Arnoldi vectors

    solver.randomize_matrix();    // deterministic sparse matrix A
    solver.run();                 // execute Arnoldi iteration

    // Print the first few basis vectors and the Hessenberg matrix
    solver.print_mat(solver.Q, solver.dim, solver.steps, "Q (orthonormal basis)");
    solver.print_mat(solver.H, solver.steps + 1, solver.steps, "H (Hessenberg)");

    return 0;
}
