/* LLM input variant 1: minimal-boundary */
#include <iostream>
#include <iomanip>
#include <cmath>

#define MAXN 30          // maximum matrix dimension (stack allocation)
#define STEPS 10         // number of Arnoldi steps (<= MAXN)

class ArnoldiSolver {
public:
    int dim;                     // matrix size
    int steps;                   // Krylov subspace dimension
    float A[MAXN][MAXN];         // input matrix
    float Q[MAXN][MAXN];         // orthonormal basis
    float H[MAXN][MAXN];         // upper Hessenberg matrix

    // Fill the matrix with a minimal deterministic pattern
    void randomize_matrix() {
        // 2x2 matrix with simple non‑zero values
        A[0][0] = 0.5f; A[0][1] = 0.2f;
        A[1][0] = 0.3f; A[1][1] = 0.7f;
        // Zero out any unused entries (not strictly needed)
        for (int r = 0; r < dim; ++r)
            for (int c = 0; c < dim; ++c)
                if (r >= 2 || c >= 2) A[r][c] = 0.0f;
    }

    float vec_norm(const float *vec) {
        float sum = 0.0f;
        for (int i = 0; i < dim; ++i) {
            float tmp = vec[i];
            sum += tmp * tmp;
        }
        return std::sqrt(sum);
    }

    float dot_product(const float *u, const float *v) {
        float acc = 0.0f;
        for (int i = 0; i < dim; ++i) {
            acc += u[i] * v[i];
        }
        return acc;
    }

    void mat_vec_mul(const float src[MAXN], float dst[MAXN]) {
        for (int i = 0; i < dim; ++i) {
            float accum = 0.0f;
            for (int j = 0; j < dim; ++j) {
                accum += A[i][j] * src[j];
            }
            dst[i] = accum;
        }
    }

    void run() {
        // deterministic start vector: all ones
        float start[MAXN];
        for (int i = 0; i < dim; ++i) start[i] = 1.0f;

        float nrm0 = vec_norm(start);
        for (int i = 0; i < dim; ++i) {
            Q[i][0] = start[i] / nrm0;
        }

        int k = 0;
        while (k < steps) {
            float w[MAXN];
            float qk[MAXN];
            for (int i = 0; i < dim; ++i) qk[i] = Q[i][k];
            mat_vec_mul(qk, w);

            for (int i = 0; i <= k; ++i) {
                float qi[MAXN];
                for (int r = 0; r < dim; ++r) qi[r] = Q[r][i];
                float hij = dot_product(qi, w);
                H[i][k] = hij;
            }

            for (int i = 0; i <= k; ++i) {
                float coeff = H[i][k];
                for (int r = 0; r < dim; ++r) {
                    w[r] -= coeff * Q[r][i];
                }
            }

            float wk_norm = vec_norm(w);
            H[k + 1][k] = wk_norm;

            if (wk_norm > 1e-6f && k + 1 < steps) {
                for (int i = 0; i < dim; ++i) {
                    Q[i][k + 1] = w[i] / wk_norm;
                }
            }
            ++k;
        }
    }

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
    ArnoldiSolver solver;
    solver.dim   = 2;            // minimal non‑trivial size
    solver.steps = 1;            // minimal number of Arnoldi steps

    solver.randomize_matrix();
    solver.run();

    solver.print_mat(solver.Q, solver.dim, solver.steps, "Q (orthonormal basis)");
    solver.print_mat(solver.H, solver.steps + 1, solver.steps, "H (Hessenberg)");
    return 0;
}
