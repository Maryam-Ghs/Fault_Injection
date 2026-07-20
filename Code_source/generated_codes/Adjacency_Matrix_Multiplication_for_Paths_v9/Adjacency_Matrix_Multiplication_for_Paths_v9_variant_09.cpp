#include <iostream>
#include <vector>
#include <cstdint>

/* LLM input variant 9: medium-deterministic-random */

class PathMatrix {
public:
    PathMatrix(int sz) : dim(sz), mat(sz, std::vector<int>(sz)) {}

    void fill_random() {
        // Deterministic linear congruential generator (LCG)
        uint32_t seed = 0xDEADBEEF; // fixed seed for reproducibility
        const uint32_t a = 1664525;
        const uint32_t c = 1013904223;
        const uint32_t m = 0xFFFFFFFF;
        int i = 0;
        while (i < dim) {
            int j = 0;
            while (j < dim) {
                seed = (a * seed + c) & m;
                mat[i][j] = static_cast<int>(seed % 2);
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
