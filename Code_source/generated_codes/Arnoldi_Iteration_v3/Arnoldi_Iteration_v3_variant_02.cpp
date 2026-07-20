/* LLM input variant 2: small-diverse */
#include <iostream>
#include <iomanip>
#include <cmath>

#define MAXN 30
#define STEPS 10

class ArnoldiSolver {
public:
    int dim;
    int steps;
    float A[MAXN][MAXN];
    float Q[MAXN][MAXN];
    float H[MAXN][MAXN];

    // Fill matrix with a small diverse deterministic pattern
    void randomize_matrix() {
        for (int r = 0; r < dim; ++r) {
            for (int c = 0; c < dim; ++c) {
                A[r][c] = static_cast<float>((r * 5 + c + 1)) * 0.01f; // values 0.01,0.02,...
            }
        }
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
        for (int i = 0; i < dim; ++i) acc += u[i] * v[i];
        return acc;
    }

    void mat_vec_mul(const float src[MAXN], float dst[MAXN]) {
        for (int i = 0; i < dim; ++i) {
            float accum = 0.0f;
            for (int j = 0; j < dim; ++j) accum += A[i][j] * src[j];
            dst[i] = accum;
        }
    }

    void run() {
        // deterministic start vector: scaled sequence
        float start[MAXN];
        for (int i = 0; i < dim; ++i) start[i] = static_cast<float>(i + 1) * 0.1f;

        float nrm0 = vec_norm(start);
        for (int i = 0; i < dim; ++i) Q[i][0] = start[i] / nrm0;

        int k = 0;
        while (k < steps) {
            float w[MAXN];
            float qk[MAXN];
            for (int i = 0; i < dim; ++i) qk[i] = Q[i][k];
            mat_vec_mul(qk, w);

            for (int i = 0; i <= k; ++i) {
                float qi[MAXN];
                for (int r = 0; r < dim; ++r) qi[r] = Q[r][i];
                H[i][k] = dot_product(qi, w);
            }

            for (int i = 0; i <= k; ++i) {
                float coeff = H[i][k];
                for (int r = 0; r < dim; ++r) w[r] -= coeff * Q[r][i];
            }

            float wk_norm = vec_norm(w);
            H[k + 1][k] = wk_norm;

            if (wk_norm > 1e-6f && k + 1 < steps) {
                for (int i = 0; i < dim; ++i) Q[i][k + 1] = w[i] / wk_norm;
            }
            ++k;
        }
    }

    void print_mat(const float mat[MAXN][MAXN], int rows, int cols, const char *name) {
        std::cout << name << " =\n";
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c)
                std::cout << std::setw(10) << std::fixed << std::setprecision(4) << mat[r][c];
            std::cout << '\n';
        }
        std::cout << '\n';
    }
};

int main() {
    ArnoldiSolver solver;
    solver.dim   = 5;
    solver.steps = 3;

    solver.randomize_matrix();
    solver.run();

    solver.print_mat(solver.Q, solver.dim, solver.steps, "Q (orthonormal basis)");
    solver.print_mat(solver.H, solver.steps + 1, solver.steps, "H (Hessenberg)");
    return 0;
}
