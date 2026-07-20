#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

/* LLM input variant 3: zeros-and-ones */

class PathMatrix {
public:
    PathMatrix(int sz) : dim(sz), mat(sz, std::vector<int>(sz)) {}

    void fill_random() {
        int i = 0;
        while (i < dim) {
            int j = 0;
            while (j < dim) {
                // deterministic identity pattern: 1 on diagonal, 0 elsewhere
                mat[i][j] = (i == j) ? 1 : 0;
                ++j;
            }
            ++i;
        }
    }

    void fill_zero() {
        int i = 0;
        while (i < dim) {
            int j = 0;
            while (j < dim) {
                mat[i][j] = 0;
                ++j;
            }
            ++i;
        }
    }

    PathMatrix multiply(const PathMatrix& other) const {
        PathMatrix prod(dim);
        int r = 0;
        while (r < dim) {
            int c = 0;
            while (c < dim) {
                int acc = 0;
                int k = 0;
                while (k < dim) {
                    acc = acc + (mat[r][k] * other.mat[k][c]);
                    ++k;
                }
                prod.mat[r][c] = acc;
                ++c;
            }
            ++r;
        }
        return prod;
    }

    void display(const char* label) const {
        std::cout << label << ":\n";
        int i = 0;
        while (i < dim) {
            int j = 0;
            while (j < dim) {
                std::cout << mat[i][j] << ' ';
                ++j;
            }
            std::cout << '\n';
            ++i;
        }
        std::cout << '\n';
    }

private:
    int dim;
    std::vector<std::vector<int>> mat;
};

int main() {
    const int size = 6;                     // medium sized matrix
    PathMatrix adjA(size), adjB(size);

    adjA.fill_random();                     // identity matrix (ones on diagonal)
    adjB.fill_zero();                       // zero matrix

    adjA.display("Adjacency Matrix A");
    adjB.display("Adjacency Matrix B");

    PathMatrix paths = adjA.multiply(adjB); // paths of length two

    paths.display("A * B (paths of length 2)");

    return 0;
}
