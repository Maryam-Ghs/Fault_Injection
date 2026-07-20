#include <iostream>
#include <vector>
#include <cmath>

/* LLM input variant 3: zeros-and-ones */

class AdjSpectrum {
public:
    // size of the matrix
    int sz;
    // adjacency matrix
    std::vector<std::vector<float>> mat;

    AdjSpectrum(int n) {
        sz = n;
        mat.assign(sz, std::vector<float>(sz, 0.0f));
        generate_deterministic();
    }

    void generate_deterministic() {
        // deterministic empty graph (all zeros, no self‑loops)
        for (int i = 0; i < sz; ++i) {
            for (int j = i + 1; j < sz; ++j) {
                mat[i][j] = 0.0f;
                mat[j][i] = 0.0f;
            }
        }
    }

    // matrix multiplication: C = A * B
    std::vector<std::vector<float>> mul(const std::vector<std::vector<float>>& A,
                                        const std::vector<std::vector<float>>& B) {
        std::vector<std::vector<float>> C(sz, std::vector<float>(sz, 0.0f));
        for (int i = 0; i < sz; ++i) {
            for (int k = 0; k < sz; ++k) {
                float aik = A[i][k];
                for (int j = 0; j < sz; ++j) {
                    C[i][j] += aik * B[k][j];
                }
            }
        }
        return C;
    }

    // QR decomposition using classical Gram‑Schmidt
    void qr_decompose(const std::vector<std::vector<float>>& A,
                      std::vector<std::vector<float>>& Q,
                      std::vector<std::vector<float>>& R) {
        Q.assign(sz, std::vector<float>(sz, 0.0f));
        R.assign(sz, std::vector<float>(sz, 0.0f));

        // copy columns of A into temporary vectors
        std::vector<std::vector<float>> a_cols(sz, std::vector<float>(sz, 0.0f));
        for (int j = 0; j < sz; ++j)
            for (int i = 0; i < sz; ++i)
                a_cols[j][i] = A[i][j];

        std::vector<std::vector<float>> u(sz, std::vector<float>(sz, 0.0f));

        for (int i = 0; i < sz; ++i) {
            // start with a copy of the current column
            for (int k = 0; k < sz; ++k)
                u[i][k] = a_cols[i][k];

            // subtract projections onto previous q vectors
            for (int j = 0; j < i; ++j) {
                float dot = 0.0f;
                for (int k = 0; k < sz; ++k)
                    dot += a_cols[i][k] * Q[k][j];
                R[j][i] = dot;
                for (int k = 0; k < sz; ++k)
                    u[i][k] -= dot * Q[k][j];
            }

            // compute norm
            float norm = 0.0f;
            for (int k = 0; k < sz; ++k)
                norm += u[i][k] * u[i][k];
            norm = std::sqrt(norm);
            R[i][i] = norm;

            // normalize to obtain q_i
            for (int k = 0; k < sz; ++k)
                Q[k][i] = u[i][k] / norm;
        }
    }

    // QR algorithm to approximate eigenvalues
    std::vector<float> compute_spectrum() {
        std::vector<std::vector<float>> A = mat;
        std::vector<std::vector<float>> Q(sz, std::vector<float>(sz));
        std::vector<std::vector<float>> R(sz, std::vector<float>(sz));

        const int max_iter = 150;
        for (int it = 0; it < max_iter; ++it) {
            qr_decompose(A, Q, R);
            A = mul(R, Q);               // A_{k+1} = R_k * Q_k
        }

        // diagonal of final A holds approximated eigenvalues
        std::vector<float> eig(sz);
        for (int i = 0; i < sz; ++i)
            eig[i] = A[i][i];
        return eig;
    }

    void print_matrix() {
        std::cout << "Adjacency matrix (" << sz << "x" << sz << "):\n";
        for (int i = 0; i < sz; ++i) {
            for (int j = 0; j < sz; ++j)
                std::cout << mat[i][j] << ' ';
            std::cout << '\n';
        }
    }

    void print_spectrum(const std::vector<float>& eig) {
        std::cout << "Approximate adjacency spectrum:\n";
        for (int i = 0; i < sz; ++i)
            std::cout << eig[i] << ' ';
        std::cout << '\n';
    }
};

int main() {
    // deterministic small graph (5 vertices) emphasizing zeros
    int n = 5;
    AdjSpectrum solver(n);

    solver.print_matrix();

    std::vector<float> spectrum = solver.compute_spectrum();

    solver.print_spectrum(spectrum);

    return 0;
}
