#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

/* LLM input variant 7: reverse-adversarial */

class PathMatrix {
public:
    PathMatrix(int sz) : dim(sz), mat(sz, std::vector<int>(sz)) {}

    // Fill matrix with the worst‑case pattern for path counting:
    // all entries are 1, generated in reverse order to stress any
    // assumptions about input ordering.
    void fill_reverse_adversarial() {
        int i = dim - 1;
        while (i >= 0) {
            int j = dim - 1;
            while (j >= 0) {
                mat[i][j] = 1;
                --j;
            }
            --i;
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

    const int size = 10;                     // larger matrix to amplify worst‑case behavior
    PathMatrix adjA(size), adjB(size);

    adjA.fill_reverse_adversarial();         // first adjacency matrix (all 1s)
    adjB.fill_reverse_adversarial();         // second adjacency matrix (all 1s)

    adjA.display("Adjacency Matrix A");
    adjB.display("Adjacency Matrix B");

    PathMatrix paths = adjA.multiply(adjB); // paths of length two

    paths.display("A * B (paths of length 2)");

    return 0;
}
