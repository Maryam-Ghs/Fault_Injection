#include <iostream>
#include <iomanip>

/* LLM input variant 3: zeros-and-ones */

class AlphaEngine {
public:
    // Fixed size for deterministic test vectors
    void run() {
        // Stack‑allocated input matrices (deterministic values)
        float mA[4][4];
        float mB[4][4];
        float mC[4][4];

        // Initialize matrices emphasizing zeros and ones
        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                mA[r][c] = (r == c) ? 1.0f : 0.0f;   // Identity matrix: 1 on diagonal, 0 elsewhere
                mB[r][c] = 1.0f;                     // All‑ones matrix
                mC[r][c] = 0.0f;                     // result matrix cleared
            }
        }

        // Manual loop‑unrolled matrix multiplication (4×4)
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                float acc = 0.0f;               // accumulator for C(i,j)

                // Unroll the inner dimension (k) by 2
                for (int k = 0; k < 4; k += 2) {
                    // Reordered arithmetic: multiply first, then add the two products
                    float p0 = mA[i][k]   * mB[k][j];
                    float p1 = mA[i][k+1] * mB[k+1][j];
                    acc = acc + p0 + p1;       // addition order changed from typical
                }

                mC[i][j] = acc;                 // store the computed element
            }
        }

        // Output the resulting matrix
        std::cout << "Result matrix C (4x4):\n";
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
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
