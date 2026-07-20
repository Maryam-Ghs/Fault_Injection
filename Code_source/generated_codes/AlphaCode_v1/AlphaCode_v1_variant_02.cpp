#include <iostream>
#include <iomanip>

/* LLM input variant 2: small-diverse */

class AlphaEngine {
public:
    // Fixed size for deterministic test vectors
    void run() {
        // Stack‑allocated input matrices (deterministic values)
        float mA[4][4];
        float mB[4][4];
        float mC[4][4];

        // Initialize matrices with a more diverse pattern:
        // mA contains a mix of positive, zero, and negative values.
        // mB includes values that wrap around a different modulus.
        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                // Example pattern: alternating sign and scaling
                mA[r][c] = ((r + c) % 2 == 0 ? 1 : -1) * static_cast<float>((r + 1) * (c + 2));
                // Example pattern: values from 2 to 8 with a modulus of 7
                mB[r][c] = static_cast<float>(((r * 3 + c * 2) % 7) + 2);
                mC[r][c] = 0.0f; // result matrix cleared
            }
        }

        // Manual loop‑unrolled matrix multiplication (4×4)
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                float acc = 0.0f; // accumulator for C(i,j)

                // Unroll the inner dimension (k) by 2
                for (int k = 0; k < 4; k += 2) {
                    // Reordered arithmetic: multiply first, then add the two products
                    float p0 = mA[i][k]   * mB[k][j];
                    float p1 = mA[i][k+1] * mB[k+1][j];
                    acc = acc + p0 + p1; // addition order changed from typical
                }

                mC[i][j] = acc; // store the computed element
            }
        }

        // Output the resulting matrix
        std::cout << "Result matrix C (4x4):\n";
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                std::cout << std::setw(12) << mC[i][j];
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
