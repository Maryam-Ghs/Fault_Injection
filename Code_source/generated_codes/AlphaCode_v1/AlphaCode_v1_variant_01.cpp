/* LLM input variant 1: minimal-boundary */
#include <iostream>
#include <iomanip>

class AlphaEngine {
public:
    // Fixed size for deterministic test vectors (minimal nontrivial size)
    void run() {
        const int N = 2;                 // minimal even dimension to keep unroll by 2
        float mA[N][N];
        float mB[N][N];
        float mC[N][N];

        // Initialize matrices with simple patterns
        for (int r = 0; r < N; ++r) {
            for (int c = 0; c < N; ++c) {
                mA[r][c] = static_cast<float>(r * N + c + 1);          // 1 .. 4
                mB[r][c] = static_cast<float>((r + c) % 5 + 1);        // 1 .. 5 repeating
                mC[r][c] = 0.0f;                                         // result matrix cleared
            }
        }

        // Manual loop‑unrolled matrix multiplication (NxN)
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                float acc = 0.0f;               // accumulator for C(i,j)

                // Unroll the inner dimension (k) by 2
                for (int k = 0; k < N; k += 2) {
                    // Reordered arithmetic: multiply first, then add the two products
                    float p0 = mA[i][k]   * mB[k][j];
                    float p1 = mA[i][k+1] * mB[k+1][j];
                    acc = acc + p0 + p1;       // addition order changed from typical
                }

                mC[i][j] = acc;                 // store the computed element
            }
        }

        // Output the resulting matrix
        std::cout << "Result matrix C (2x2):\n";
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                std::cout << std::setw(8) << mC[i][j];
            }
            std::cout << '\n';
        }
    }
};

int main() {
    AlphaEngine solver;
    solver.run();
    return 0;
}
