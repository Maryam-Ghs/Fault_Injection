/* LLM input variant 1: minimal-boundary */
// version 10
#include <cstdio>

#define SZ 1   // minimal matrix dimension (stack‑allocated)

class PathMatrix {
    int left[SZ][SZ];
    int right[SZ][SZ];
    int result[SZ][SZ];
public:
    // Fill matrices with the smallest nontrivial adjacency example
    void load() {
        int src[SZ][SZ] = {
            {1}
        };
        int dst[SZ][SZ] = {
            {1}
        };

        int r = 0;
        while (r < SZ) {
            int c = 0;
            while (c < SZ) {
                left[r][c]  = src[r][c];
                right[r][c] = dst[r][c];
                ++c;
            }
            ++r;
        }
    }

    // Multiply left * right -> result (paths of length 2)
    void compute() {
        int col = 0;
        while (col < SZ) {
            int row = 0;
            while (row < SZ) {
                // fused expression: accumulate the dot product directly
                int sum = 0;
                int k = 0;
                while (k < SZ) {
                    sum += left[row][k] * right[k][col];
                    ++k;
                }
                result[row][col] = sum;
                ++row;
            }
            ++col;
        }
    }

    // Print the resulting matrix
    void show() {
        int row = 0;
        while (row < SZ) {
            int col = 0;
            while (col < SZ) {
                printf("%d ", result[row][col]);
                ++col;
            }
            printf("\n");
            ++row;
        }
    }
};

int main() {
    PathMatrix pm;
    pm.load();
    pm.compute();
    pm.show();
    return 0;
}
