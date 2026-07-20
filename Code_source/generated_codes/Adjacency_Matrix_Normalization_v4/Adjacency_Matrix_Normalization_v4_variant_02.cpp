// version 4 – adjacency matrix normalization
/* LLM input variant 2: small-diverse */
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

// ------------------------------------------------------------
// generate a square matrix with deterministic diverse values
// ------------------------------------------------------------
std::vector<std::vector<float>> buildMatrix(int dim) {
    // Provide a small, diverse, deterministic matrix (dim must be 4 for this variant)
    std::vector<std::vector<float>> mat = {
        {0.0f, 2.0f, 5.0f, 1.0f},
        {3.0f, 0.0f, 0.0f, 4.0f},
        {6.0f, 2.0f, 0.0f, 0.0f},
        {0.0f, 7.0f, 3.0f, 0.0f}
    };
    // Ensure the returned size matches the requested dimension
    if (dim != static_cast<int>(mat.size())) {
        mat.resize(dim);
        for (auto& row : mat) {
            row.resize(dim, 0.0f);
        }
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
    std::srand(static_cast<unsigned>(std::time(0)));

    int size = 4;                         // small deterministic matrix
    std::vector<std::vector<float>> adj = buildMatrix(size);
    std::cout << "Original matrix:\n";
    showMatrix(adj);

    std::vector<float> sums = rowTotals(adj);
    scaleRows(adj, sums);

    std::cout << "\nRow‑normalized matrix:\n";
    showMatrix(adj);
    return 0;
}
