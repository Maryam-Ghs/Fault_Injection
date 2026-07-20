/* LLM input variant 10: large-safe-stress */
// version 10
#include <cstdio>

#define SZ 50   // larger matrix dimension (stack‑allocated)

class PathMatrix {
    int left[SZ][SZ];
    int right[SZ][SZ];
    int result[SZ][SZ];
public:
    // Fill matrices with a larger deterministic adjacency example (directed cycle)
    void load() {
        for (int r = 0; r < SZ; ++r) {
            for (int c = 0; c < SZ; ++c) {
                left[r][c] = (c == (r + 1) % SZ) ? 1 : 0;
                right[r][c] = (c == (r + 1) % SZ) ? 1 : 0;
            }
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
