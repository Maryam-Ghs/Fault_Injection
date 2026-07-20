#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

/* LLM input variant 4: signed-extremes */

class PathMatrix {
public:
    PathMatrix(int sz) : dim(sz), mat(sz, std::vector<int>(sz)) {}

    void fill_random() {
        int i = 0;
        while (i < dim) {
            int j = 0;
            while (j < dim) {
                // deterministic mix of -2, -1, 0, 1, 2
                mat[i][j] = ((i * dim + j) % 5) - 2;
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
    std::srand(static_cast<unsigned>(std::time(0)));

    const int size = 6;                     // medium sized matrix
    PathMatrix adjA(size), adjB(size);

    adjA.fill_random();                     // first adjacency matrix with signed extremes
    adjB.fill_random();                     // second adjacency matrix with signed extremes

    adjA.display("Adjacency Matrix A");
    adjB.display("Adjacency Matrix B");

    PathMatrix paths = adjA.multiply(adjB); // paths of length two

    paths.display("A * B (paths of length 2)");

    return 0;
}
