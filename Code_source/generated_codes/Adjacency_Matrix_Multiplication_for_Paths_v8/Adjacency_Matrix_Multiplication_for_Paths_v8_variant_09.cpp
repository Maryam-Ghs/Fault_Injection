/* LLM input variant 9: medium-deterministic-random */
#include <iostream>
#include <cstdlib>

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

/* Deterministic pseudo‑random fill (0 or 1) */
void detFill(int** m, int dim) {
    unsigned int seed = 123456789u;
    int r = 0;
    while (r < dim) {
        int c = 0;
        while (c < dim) {
            seed = seed * 1103515245u + 12345u;
            m[r][c] = (seed >> 16) & 1u;   // use a single bit
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
            c[i][j] = 0;
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
                c[i][l] = c[i][l] + aik * b[k][l];
                ++l;
            }
            ++k;
        }
        ++i;
    }
}

int main() {
    int n = 7;                     // size of the graph (medium deterministic)
    int** adj   = createMat(n);
    int** paths = createMat(n);

    detFill(adj, n);

    std::cout << "Adjacency matrix:\n";
    showMat(adj, n);

    mulMat(adj, adj, paths, n);

    std::cout << "Paths of length 2 (A*A):\n";
    showMat(paths, n);

    delMat(adj, n);
    delMat(paths, n);
    return 0;
}
