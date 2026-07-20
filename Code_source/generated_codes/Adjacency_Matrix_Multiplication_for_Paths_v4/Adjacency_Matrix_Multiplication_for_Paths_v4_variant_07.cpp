/* LLM input variant 7: reverse-adversarial */
// Adjacency Matrix Multiplication for Paths – version #4
// Implements A * A to count length‑2 paths.
// Uses heap allocation, branch‑minimal loops and reordered arithmetic.

#include <iostream>
#include <iomanip>

// ---------- Helper: allocate a square matrix ----------
int** allocMat(int sz) {
    int** mat = new int*[sz];
    for (int i = 0; i < sz; ++i) {
        mat[i] = new int[sz];
    }
    return mat;
}

// ---------- Helper: release a square matrix ----------
void freeMat(int** mat, int sz) {
    for (int i = 0; i < sz; ++i) {
        delete[] mat[i];
    }
    delete[] mat;
}

// ---------- Helper: fill matrix with a deterministic reverse‑ordered dense test pattern ----------
void fillMat(int** mat, int sz) {
    // Create a lower‑triangular matrix (edges from higher‑indexed nodes to lower‑indexed nodes)
    // i.e., mat[i][j] = 1 if i > j, otherwise 0.
    for (int r = 0; r < sz; ++r) {
        for (int c = 0; c < sz; ++c) {
            mat[r][c] = (r > c) ? 1 : 0;
        }
    }
}

// ---------- Helper: matrix multiplication (A * B) ----------
int** mulMat(int** left, int** right, int sz) {
    int** prod = allocMat(sz);
    for (int i = 0; i < sz; ++i) {
        for (int j = 0; j < sz; ++j) {
            int acc = 0;
            // Reordered arithmetic: right first, then left
            for (int k = 0; k < sz; ++k) {
                acc += right[k][j] * left[i][k];
            }
            prod[i][j] = acc;
        }
    }
    return prod;
}

// ---------- Helper: print matrix ----------
void showMat(int** mat, int sz) {
    for (int i = 0; i < sz; ++i) {
        for (int j = 0; j < sz; ++j) {
            std::cout << std::setw(3) << mat[i][j] << ' ';
        }
        std::cout << '\n';
    }
}

// ---------- Main ----------
int main() {
    const int dim = 6;                     // deterministic size (larger than original)
    int** adj = allocMat(dim);              // original adjacency matrix
    fillMat(adj, dim);

    std::cout << "Adjacency matrix (A):\n";
    showMat(adj, dim);
    std::cout << "\n";

    int** paths2 = mulMat(adj, adj, dim);   // A squared → length‑2 paths

    std::cout << "A² (paths of length 2):\n";
    showMat(paths2, dim);
    std::cout << "\n";

    freeMat(adj, dim);
    freeMat(paths2, dim);
    return 0;
}
