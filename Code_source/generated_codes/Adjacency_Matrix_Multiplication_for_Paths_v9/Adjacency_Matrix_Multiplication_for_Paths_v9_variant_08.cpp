/* LLM input variant 8: sparse-skewed */
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

class PathMatrix {
public:
    PathMatrix(int sz) : dim(sz), mat(sz, std::vector<int>(sz)) {}

    void fill_random() {
        int i = 0;
        while (i < dim) {
            int j = 0;
            while (j < dim) {
                // Sparse matrix: low probability of 1s, higher in first row/column
                int prob = (i == 0 || j == 0) ? 30 : 5; // percentage chance (out of 100)
                mat[i][j] = (std::rand() % 100) < prob ? 1 : 0;
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
                    // reordered arithmetic: b*a + d*c instead of a*b + c*d
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
    std::srand(42); // deterministic seed for reproducible sparse-skewed input

    const int size = 6;                     // medium sized matrix
    PathMatrix adjA(size), adjB(size);

    adjA.fill_random();                     // first adjacency matrix
    adjB.fill_random();                     // second adjacency matrix (could be same as A)

    adjA.display("Adjacency Matrix A");
    adjB.display("Adjacency Matrix B");

    PathMatrix paths = adjA.multiply(adjB); // paths of length two

    paths.display("A * B (paths of length 2)");

    return 0;
}
