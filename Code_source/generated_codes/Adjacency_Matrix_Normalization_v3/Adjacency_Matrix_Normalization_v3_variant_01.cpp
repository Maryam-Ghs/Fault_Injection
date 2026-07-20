#include <iostream>
#include <iomanip>

/* LLM input variant 1: minimal-boundary */

class AdjNorm {
public:
    void run() {
        const int N = 1; // minimal size
        int raw[1][1] = {
            {0}
        };

        float mat[1][1];
        // manual copy – loop unrolled for N=1
        mat[0][0] = static_cast<float>(raw[0][0]);

        std::cout << "Original matrix:\n";
        print(mat, N);

        // ----- row‑sum computation (expanded for N=1) -----
        float rowSum0 = mat[0][0]; // only element in the row

        // ----- normalization – in‑place, loop unrolled for N=1 -----
        mat[0][0] = (rowSum0 != 0.0f) ? mat[0][0] / rowSum0 : 0.0f;

        std::cout << "\nRow‑normalized matrix:\n";
        print(mat, N);
    }

private:
    // printing uses while loops (different loop structure)
    void print(float a[][1], int size) {
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
