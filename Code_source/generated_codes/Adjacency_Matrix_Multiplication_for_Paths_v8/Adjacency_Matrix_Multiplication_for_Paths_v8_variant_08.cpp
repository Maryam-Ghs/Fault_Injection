#include <iostream>
#include <cstdlib>
#include <ctime>

/* LLM input variant 8: sparse-skewed */

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

/* Deterministic sparse and skewed fill:
   - First two rows contain a few edges.
   - Remaining rows are all zeros. */
void randFill(int** m, int dim) {
    int r = 0;
    while (r < dim) {
        int c = 0;
        while (c < dim) {
            if (r == 0 && (c == 1 || c == 3)) {
                m[r][c] = 1;
            } else if (r == 1 && (c == 2 || c == 4)) {
                m[r][c] = 1;
            } else {
                m[r][c] = 0;
            }
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
    std::srand(static_cast<unsigned>(std::time(0)));

    int n = 8;                     // larger graph, still sparse
    int** adj   = createMat(n);
    int** paths = createMat(n);

    randFill(adj, n);

    std::cout << "Adjacency matrix:\n";
    showMat(adj, n);

    mulMat(adj, adj, paths, n);

    std::cout << "Paths of length 2 (A*A):\n";
    showMat(paths, n);

    delMat(adj, n);
    delMat(paths, n);
    return 0;
}
