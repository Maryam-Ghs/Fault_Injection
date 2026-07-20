#include <iostream>
#include <iomanip>

/* LLM input variant 9: medium-deterministic-random */

class AdjNorm {
public:
    void run() {
        // medium-sized predefined adjacency matrix (int values)
        int raw[5][5] = {
            {0, 12, 7, 3, 15},
            {8, 0, 14, 2, 9},
            {5, 11, 0, 6, 13},
            {4, 10, 2, 0, 1},
            {9, 3, 8, 7, 0}
        };

        // convert to float matrix on the stack
        float mat[5][5];
        // manual copy – loop unrolled
        mat[0][0] = (float)raw[0][0];
        mat[0][1] = (float)raw[0][1];
        mat[0][2] = (float)raw[0][2];
        mat[0][3] = (float)raw[0][3];
        mat[0][4] = (float)raw[0][4];
        mat[1][0] = (float)raw[1][0];
        mat[1][1] = (float)raw[1][1];
        mat[1][2] = (float)raw[1][2];
        mat[1][3] = (float)raw[1][3];
        mat[1][4] = (float)raw[1][4];
        mat[2][0] = (float)raw[2][0];
        mat[2][1] = (float)raw[2][1];
        mat[2][2] = (float)raw[2][2];
        mat[2][3] = (float)raw[2][3];
        mat[2][4] = (float)raw[2][4];
        mat[3][0] = (float)raw[3][0];
        mat[3][1] = (float)raw[3][1];
        mat[3][2] = (float)raw[3][2];
        mat[3][3] = (float)raw[3][3];
        mat[3][4] = (float)raw[3][4];
        mat[4][0] = (float)raw[4][0];
        mat[4][1] = (float)raw[4][1];
        mat[4][2] = (float)raw[4][2];
        mat[4][3] = (float)raw[4][3];
        mat[4][4] = (float)raw[4][4];

        std::cout << "Original matrix:\n";
        print(mat);

        // ----- row‑sum computation (expanded multi‑step) -----
        float rowSum0 = mat[0][0] + mat[0][1] + mat[0][2] + mat[0][3] + mat[0][4];
        float rowSum1 = mat[1][0] + mat[1][1] + mat[1][2] + mat[1][3] + mat[1][4];
        float rowSum2 = mat[2][0] + mat[2][1] + mat[2][2] + mat[2][3] + mat[2][4];
        float rowSum3 = mat[3][0] + mat[3][1] + mat[3][2] + mat[3][3] + mat[3][4];
        float rowSum4 = mat[4][0] + mat[4][1] + mat[4][2] + mat[4][3] + mat[4][4];

        // ----- normalization – in‑place, loop unrolled -----
        mat[0][0] = (rowSum0 != 0.0f) ? mat[0][0] / rowSum0 : 0.0f;
        mat[0][1] = (rowSum0 != 0.0f) ? mat[0][1] / rowSum0 : 0.0f;
        mat[0][2] = (rowSum0 != 0.0f) ? mat[0][2] / rowSum0 : 0.0f;
        mat[0][3] = (rowSum0 != 0.0f) ? mat[0][3] / rowSum0 : 0.0f;
        mat[0][4] = (rowSum0 != 0.0f) ? mat[0][4] / rowSum0 : 0.0f;

        mat[1][0] = (rowSum1 != 0.0f) ? mat[1][0] / rowSum1 : 0.0f;
        mat[1][1] = (rowSum1 != 0.0f) ? mat[1][1] / rowSum1 : 0.0f;
        mat[1][2] = (rowSum1 != 0.0f) ? mat[1][2] / rowSum1 : 0.0f;
        mat[1][3] = (rowSum1 != 0.0f) ? mat[1][3] / rowSum1 : 0.0f;
        mat[1][4] = (rowSum1 != 0.0f) ? mat[1][4] / rowSum1 : 0.0f;

        mat[2][0] = (rowSum2 != 0.0f) ? mat[2][0] / rowSum2 : 0.0f;
        mat[2][1] = (rowSum2 != 0.0f) ? mat[2][1] / rowSum2 : 0.0f;
        mat[2][2] = (rowSum2 != 0.0f) ? mat[2][2] / rowSum2 : 0.0f;
        mat[2][3] = (rowSum2 != 0.0f) ? mat[2][3] / rowSum2 : 0.0f;
        mat[2][4] = (rowSum2 != 0.0f) ? mat[2][4] / rowSum2 : 0.0f;

        mat[3][0] = (rowSum3 != 0.0f) ? mat[3][0] / rowSum3 : 0.0f;
        mat[3][1] = (rowSum3 != 0.0f) ? mat[3][1] / rowSum3 : 0.0f;
        mat[3][2] = (rowSum3 != 0.0f) ? mat[3][2] / rowSum3 : 0.0f;
        mat[3][3] = (rowSum3 != 0.0f) ? mat[3][3] / rowSum3 : 0.0f;
        mat[3][4] = (rowSum3 != 0.0f) ? mat[3][4] / rowSum3 : 0.0f;

        mat[4][0] = (rowSum4 != 0.0f) ? mat[4][0] / rowSum4 : 0.0f;
        mat[4][1] = (rowSum4 != 0.0f) ? mat[4][1] / rowSum4 : 0.0f;
        mat[4][2] = (rowSum4 != 0.0f) ? mat[4][2] / rowSum4 : 0.0f;
        mat[4][3] = (rowSum4 != 0.0f) ? mat[4][3] / rowSum4 : 0.0f;
        mat[4][4] = (rowSum4 != 0.0f) ? mat[4][4] / rowSum4 : 0.0f;

        std::cout << "\nRow‑normalized matrix:\n";
        print(mat);
    }

private:
    // printing uses while loops (different loop structure)
    void print(float a[5][5]) {
        int i = 0;
        while (i < 5) {
            int j = 0;
            while (j < 5) {
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
