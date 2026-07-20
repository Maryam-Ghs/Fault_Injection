/* LLM input variant 9: medium-deterministic-random */
// version 4 – adjacency matrix normalization
#include <iostream>
#include <vector>
#include <cstdlib>

// ------------------------------------------------------------
// generate a square matrix with deterministic pseudo‑random integer weights (0‑9)
// ------------------------------------------------------------
std::vector<std::vector<float>> buildMatrix(int dim) {
    std::vector<std::vector<float>> mat(dim, std::vector<float>(dim));
    unsigned int seed = 123456789u; // fixed seed for reproducibility
    int i = 0;
    while (i < dim) {
        int j = 0;
        while (j < dim) {
            // simple linear congruential generator (LCG) for deterministic values
            seed = (seed * 1103515245u + 12345u) & 0x7fffffffu;
            int rnd = static_cast<int>(seed % 10u);
            mat[i][j] = (i == j) ? 0.0f : static_cast<float>(rnd);
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
    // deterministic input generation – no time‑based seeding
    int size = 6;                         // medium sized deterministic array
    std::vector<std::vector<float>> adj = buildMatrix(size);
    std::cout << "Original matrix:\n";
    showMatrix(adj);

    std::vector<float> sums = rowTotals(adj);
    scaleRows(adj, sums);

    std::cout << "\nRow‑normalized matrix:\n";
    showMatrix(adj);
    return 0;
}
