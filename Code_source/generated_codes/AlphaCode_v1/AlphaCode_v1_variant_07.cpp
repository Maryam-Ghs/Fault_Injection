/* LLM input variant 7: reverse-adversarial */
#include <iostream>
#include <iomanip>

class AlphaEngine {
public:
    // Fixed size for deterministic test vectors
    void run() {
        // Stack‑allocated input matrices (deterministic values)
        float mA[4][4];
        float mB[4][4];
        float mC[4][4];

        // Initialize matrices with reverse‑ordered, alternating‑sign patterns
        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                int idx = r * 4 + c;                         // 0 .. 15
                // mA: descending magnitude 16..1, alternating sign
                mA[r][c] = static_cast<float>((16 - idx) * ((idx % 2 == 0) ? 1 : -1));
                // mB: descending pattern 5..1 (mod 5), alternating sign based on (r+c)
                int modVal = 5 - ((r + c) % 5);               // 5 .. 1
                mB[r][c] = static_cast<float>(modVal * (((r + c) % 2 == 0) ? 1 : -1));
                mC[r][c] = 0.0f;                             // result matrix cleared
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
