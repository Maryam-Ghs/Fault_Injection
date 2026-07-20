#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

/* LLM input variant 10: large-safe-stress */

class PathMatrix {
public:
    PathMatrix(int sz) : dim(sz), mat(sz, std::vector<int>(sz)) {}

    // deterministic fill: pattern based on indices
    void fill_pattern() {
        int i = 0;
        while (i < dim) {
            int j = 0;
            while (j < dim) {
                mat[i][j] = (i + j) % 2;
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
    // Fixed seed for reproducibility (not used in deterministic fill)
    std::srand(static_cast<unsigned>(std::time(0)));

    const int size = 200; // larger matrix for stress testing while remaining safe
    PathMatrix adjA(size), adjB(size);

    adjA.fill_pattern(); // deterministic adjacency matrix A
    adjB.fill_pattern(); // deterministic adjacency matrix B

    adjA.display("Adjacency Matrix A");
    adjB.display("Adjacency Matrix B");

    PathMatrix paths = adjA.multiply(adjB); // paths of length two

    paths.display("A * B (paths of length 2)");

    return 0;
}
