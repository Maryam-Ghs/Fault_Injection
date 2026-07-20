#include <iostream>
#include <iomanip>

/* LLM input variant 8: sparse-skewed */

class AdjNorm {
public:
    void run() {
        const int N = 5; // increased size with sparsity
        // sparse adjacency matrix (int values)
        int raw[N][N] = {
            {0, 0, 0, 0, 9},
            {0, 0, 0, 0, 0},
            {0, 3, 0, 0, 0},
            {0, 0, 0, 0, 0},
            {2, 0, 0, 0, 0}
        };

        // convert to float matrix on the stack
        float mat[N][N];
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                mat[i][j] = static_cast<float>(raw[i][j]);
            }
        }

        std::cout << "Original matrix:\n";
        print(mat, N);

        // ----- row‑sum computation (expanded multi‑step) -----
        float rowSum[N];
        for (int i = 0; i < N; ++i) {
            float sum = 0.0f;
            for (int j = 0; j < N; ++j) {
                sum += mat[i][j];
            }
            rowSum[i] = sum;
        }

        // ----- normalization – in‑place, loop unrolled -----
        for (int i = 0; i < N; ++i) {
            float divisor = (rowSum[i] != 0.0f) ? rowSum[i] : 1.0f;
            for (int j = 0; j < N; ++j) {
                mat[i][j] = (rowSum[i] != 0.0f) ? mat[i][j] / divisor : 0.0f;
            }
        }

        std::cout << "\nRow‑normalized matrix:\n";
        print(mat, N);
    }

private:
    // printing uses while loops (different loop structure)
    void print(float a[][5], int size) {
        int i = 0;
        while (i < size) {
            int j = 0;
            while (j < size) {
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
