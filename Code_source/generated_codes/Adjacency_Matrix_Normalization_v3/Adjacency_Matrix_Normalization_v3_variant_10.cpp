#include <iostream>
#include <iomanip>

constexpr int N = 50;  // larger safe size for stress testing

/* LLM input variant 10: large-safe-stress */

class AdjNorm {
public:
    void run() {
        // deterministic larger adjacency matrix (int values)
        int raw[N][N];
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                raw[i][j] = (i == j) ? 0 : ((i + j) % 10 + 1);
            }
        }

        // convert to float matrix on the stack
        float mat[N][N];
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                mat[i][j] = static_cast<float>(raw[i][j]);
            }
        }

        std::cout << "Original matrix:\n";
        print(mat);

        // ----- row‑sum computation (expanded multi‑step) -----
        float rowSum[N];
        for (int i = 0; i < N; ++i) {
            float sum = 0.0f;
            int j = 0;
            while (j < N) {
                sum += mat[i][j];
                ++j;
            }
            rowSum[i] = sum;
        }

        // ----- normalization – in‑place, loop unrolled (now looped for safety) -----
        for (int i = 0; i < N; ++i) {
            int j = 0;
            while (j < N) {
                mat[i][j] = (rowSum[i] != 0.0f) ? mat[i][j] / rowSum[i] : 0.0f;
                ++j;
            }
        }

        std::cout << "\nRow‑normalized matrix:\n";
        print(mat);
    }

private:
    // printing uses while loops (different loop structure)
    void print(float a[N][N]) {
        int i = 0;
        while (i < N) {
            int j = 0;
            while (j < N) {
                std::cout << std::fixed << std::setprecision(3) << a[i][j] << " ";
                ++j;
            }
            std::cout << "\n";
            ++i;
        }
    }
};

int main() {
    AdjNorm demo;
    demo.run();
    return 0;
}
