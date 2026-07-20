#include <iostream>
#include <iomanip>

/* LLM input variant 5: duplicate-heavy */

class AdjNorm {
public:
    void run() {
        // small predefined adjacency matrix (int values) with many duplicates
        int raw[3][3] = {
            {0, 5, 5},
            {5, 0, 5},
            {5, 5, 0}
        };

        // convert to float matrix on the stack
        float mat[3][3];
        // manual copy – loop unrolled
        mat[0][0] = (float)raw[0][0];
        mat[0][1] = (float)raw[0][1];
        mat[0][2] = (float)raw[0][2];
        mat[1][0] = (float)raw[1][0];
        mat[1][1] = (float)raw[1][1];
        mat[1][2] = (float)raw[1][2];
        mat[2][0] = (float)raw[2][0];
        mat[2][1] = (float)raw[2][1];
        mat[2][2] = (float)raw[2][2];

        std::cout << "Original matrix:\n";
        print(mat);

        // ----- row‑sum computation (expanded multi‑step) -----
        float rowSum0 = mat[0][0] + mat[0][1] + mat[0][2];
        float rowSum1 = mat[1][0] + mat[1][1] + mat[1][2];
        float rowSum2 = mat[2][0] + mat[2][1] + mat[2][2];

        // ----- normalization – in‑place, loop unrolled -----
        mat[0][0] = (rowSum0 != 0.0f) ? mat[0][0] / rowSum0 : 0.0f;
        mat[0][1] = (rowSum0 != 0.0f) ? mat[0][1] / rowSum0 : 0.0f;
        mat[0][2] = (rowSum0 != 0.0f) ? mat[0][2] / rowSum0 : 0.0f;

        mat[1][0] = (rowSum1 != 0.0f) ? mat[1][0] / rowSum1 : 0.0f;
        mat[1][1] = (rowSum1 != 0.0f) ? mat[1][1] / rowSum1 : 0.0f;
        mat[1][2] = (rowSum1 != 0.0f) ? mat[1][2] / rowSum1 : 0.0f;

        mat[2][0] = (rowSum2 != 0.0f) ? mat[2][0] / rowSum2 : 0.0f;
        mat[2][1] = (rowSum2 != 0.0f) ? mat[2][1] / rowSum2 : 0.0f;
        mat[2][2] = (rowSum2 != 0.0f) ? mat[2][2] / rowSum2 : 0.0f;

        std::cout << "\nRow‑normalized matrix:\n";
        print(mat);
    }

private:
    // printing uses while loops (different loop structure)
    void print(float a[3][3]) {
        int i = 0;
        while (i < 3) {
            int j = 0;
            while (j < 3) {
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
