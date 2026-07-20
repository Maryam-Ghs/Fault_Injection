#include <iostream>
#include <cstdlib>
#include <ctime>

/* LLM input variant 10: large-safe-stress */

int** createMat(int dim) {
    int** m = new int*[dim];
    int i = 0;
    while (i < dim) {
        m[i] = new int[dim];
        ++i;
    }
    return m;
}

void delMat(int** m, int dim) {
    int i = 0;
    while (i < dim) {
        delete[] m[i];
        ++i;
    }
    delete[] m;
}

void randFill(int** m, int dim) {
    int r = 0;
    while (r < dim) {
        int c = 0;
        while (c < dim) {
            m[r][c] = std::rand() % 2;          // 0 or 1 edge
            ++c;
        }
        ++r;
    }
}

void showMat(int** m, int dim) {
    int r = 0;
    while (r < dim) {
        int c = 0;
        while (c < dim) {
            std::cout << m[r][c] << ' ';
            ++c;
        }
        std::cout << '\n';
        ++r;
    }
    std::cout << "----\n";
}

/* Compute C = A * B (paths of length 2) */
void mulMat(int** a, int** b, int** c, int dim) {
    int i = 0;
    while (i < dim) {
        int j = 0;
        while (j < dim) {
            c[i][j] = 0;                       // initialise
            ++j;
        }
        ++i;
    }

    i = 0;
    while (i < dim) {
        int k = 0;
        while (k < dim) {
            int aik = a[i][k];
            int l = 0;
            while (l < dim) {
                c[i][l] = c[i][l] + aik * b[k][l];   // fused accumulation
                ++l;
            }
            ++k;
        }
        ++i;
    }
}

int main() {
    std::srand(0);  // deterministic seed for reproducibility

    int n = 500;                     // larger safe size for stress testing
    int** adj   = createMat(n);
    int** paths = createMat(n);

    randFill(adj, n);

    std::cout << "Adjacency matrix (first 20 rows shown):\n";
    // Show only a subset to keep output manageable
    for (int r = 0; r < std::min(n, 20); ++r) {
        for (int c = 0; c < n; ++c) {
            std::cout << adj[r][c] << ' ';
        }
        std::cout << '\n';
    }
    std::cout << "----\n";

    mulMat(adj, adj, paths, n);

    std::cout << "Paths of length 2 (A*A) (first 20 rows shown):\n";
    for (int r = 0; r < std::min(n, 20); ++r) {
        for (int c = 0; c < n; ++c) {
            std::cout << paths[r][c] << ' ';
        }
        std::cout << '\n';
    }
    std::cout << "----\n";

    delMat(adj, n);
    delMat(paths, n);
    return 0;
}
