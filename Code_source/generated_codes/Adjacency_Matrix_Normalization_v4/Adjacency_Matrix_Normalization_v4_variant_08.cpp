/* LLM input variant 8: sparse-skewed */
// version 4 – adjacency matrix normalization
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

// ------------------------------------------------------------
// generate a square matrix with sparse, skewed integer weights (0‑9)
// ------------------------------------------------------------
std::vector<std::vector<float>> buildMatrix(int dim) {
    std::vector<std::vector<float>> mat(dim, std::vector<float>(dim));
    int i = 0;
    while (i < dim) {
        int j = 0;
        while (j < dim) {
            // diagonal forced to zero (no self‑loops)
            if (i == j) {
                mat[i][j] = 0.0f;
            } else {
                // low probability of a non‑zero entry to create sparsity
                int rnd = std::rand() % 10;               // 0‑9
                if (rnd < 2) {                            // ~20% density
                    // skew weights upward with row index (later rows larger)
                    float weight = static_cast<float>((i + 1) * (rnd + 1));
                    mat[i][j] = weight;
                } else {
                    mat[i][j] = 0.0f;
                }
            }
            ++j;
        }
        ++i;
    }
    return mat;
}

// ------------------------------------------------------------
// compute the sum of each row – stored in a simple vector
// ------------------------------------------------------------
std::vector<float> rowTotals(std::vector<std::vector<float>>& src) {
    int n = src.size();
    std::vector<float> totals(n);
    int r = 0;
    while (r < n) {
        float acc = 0.0f;
        int c = 0;
        while (c < n) {
            acc = acc + src[r][c];
            ++c;
        }
        // add a tiny epsilon to keep division safe without a branch
        totals[r] = acc + 1e-6f;
        ++r;
    }
    return totals;
}

// ------------------------------------------------------------
// normalize each row by its sum (row‑stochastic matrix)
// ------------------------------------------------------------
void scaleRows(std::vector<std::vector<float>>& mat,
               std::vector<float>& sums) {
    int n = mat.size();
    int r = 0;
    while (r < n) {
        // reciprocal of the (non‑zero) row sum
        float inv = 1.0f / sums[r];
        int c = 0;
        while (c < n) {
            // expanded arithmetic: multiply by reciprocal instead of divide
            float tmp = mat[r][c] * inv;
            mat[r][c] = tmp;
            ++c;
        }
        ++r;
    }
}

// ------------------------------------------------------------
// pretty‑print the matrix
// ------------------------------------------------------------
void showMatrix(std::vector<std::vector<float>>& m) {
    int n = m.size();
    int i = 0;
    while (i < n) {
        int j = 0;
        while (j < n) {
            std::cout << m[i][j] << ' ';
            ++j;
        }
        std::cout << '\n';
        ++i;
    }
}

// ------------------------------------------------------------
int main() {
    // fixed seed for deterministic, sparse‑skewed data
    std::srand(42);

    int size = 12;                         // larger matrix to accentuate sparsity
    std::vector<std::vector<float>> adj = buildMatrix(size);
    std::cout << "Original matrix:\n";
    showMatrix(adj);

    std::vector<float> sums = rowTotals(adj);
    scaleRows(adj, sums);

    std::cout << "\nRow‑normalized matrix:\n";
    showMatrix(adj);
    return 0;
}
