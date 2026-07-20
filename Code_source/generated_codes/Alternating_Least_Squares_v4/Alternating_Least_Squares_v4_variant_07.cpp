#include <iostream>
#include <iomanip>

#define ROWS 5       // number of users
#define COLS 6       // number of items
#define FACT 2       // latent factors
#define ITER 8       // ALS iterations
#define LAMB 0.1f    // regularization

/* LLM input variant 7: reverse-adversarial */

int main() {
    // ----- generate rating matrix (ROW x COL) -----
    // Reverse-ordered dense ratings: each row contains 5,4,3,2,1,0
    float R[ROWS][COLS];
    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c < COLS; ++c) {
            R[r][c] = static_cast<float>(5 - (COLS - 1 - c));
        }
    }

    // ----- factor matrices: USER (ROW x FACT) & ITEM (COL x FACT) -----
    // Awkward descending initialization to stress the solver
    float U[ROWS][FACT];
    float V[COLS][FACT];
    for (int i = 0; i < ROWS; ++i) {
        for (int f = 0; f < FACT; ++f) {
            U[i][f] = 1.0f - 0.1f * static_cast<float>((i * FACT + f) % 10);
        }
    }
    for (int j = 0; j < COLS; ++j) {
        for (int f = 0; f < FACT; ++f) {
            V[j][f] = 1.0f - 0.1f * static_cast<float>((j * FACT + f) % 10);
        }
    }

    // ----- ALS core -----
    for (int it = 0; it < ITER; ++it) {
        // ---- update USER factors (U) ----
        for (int i = 0; i < ROWS; ++i) {
            float a00 = LAMB, a01 = 0.0f, a11 = LAMB;
            float b0 = 0.0f, b1 = 0.0f;

            int j = 0;
            for (; j <= COLS - 2; j += 2) {
                float rj0 = R[i][j];
                float rj1 = R[i][j + 1];

                float vj0_0 = V[j][0];
                float vj0_1 = V[j][1];
                a00 += vj0_0 * vj0_0;
                a01 += vj0_0 * vj0_1;
                a11 += vj0_1 * vj0_1;

                float vj1_0 = V[j + 1][0];
                float vj1_1 = V[j + 1][1];
                a00 += vj1_0 * vj1_0;
                a01 += vj1_0 * vj1_1;
                a11 += vj1_1 * vj1_1;

                b0 += vj0_0 * rj0 + vj1_0 * rj1;
                b1 += vj0_1 * rj0 + vj1_1 * rj1;
            }
            for (; j < COLS; ++j) {
                float rij = R[i][j];
                float vj0 = V[j][0];
                float vj1 = V[j][1];
                a00 += vj0 * vj0;
                a01 += vj0 * vj1;
                a11 += vj1 * vj1;
                b0 += vj0 * rij;
                b1 += vj1 * rij;
            }

            float a10 = a01;
            float det = a00 * a11 - a01 * a10;
            if (det == 0.0f) det = 1e-6f;
            float inv00 =  a11 / det;
            float inv01 = -a01 / det;
            float inv10 = -a10 / det;
            float inv11 =  a00 / det;

            float new0 = inv00 * b0 + inv01 * b1;
            float new1 = inv10 * b0 + inv11 * b1;
            U[i][0] = new0;
            U[i][1] = new1;
        }

        // ---- update ITEM factors (V) ----
        for (int j = 0; j < COLS; ++j) {
            float a00 = LAMB, a01 = 0.0f, a11 = LAMB;
            float b0 = 0.0f, b1 = 0.0f;

            int i = 0;
            for (; i <= ROWS - 2; i += 2) {
                float rij0 = R[i][j];
                float rij1 = R[i + 1][j];

                float ui0_0 = U[i][0];
                float ui0_1 = U[i][1];
                a00 += ui0_0 * ui0_0;
                a01 += ui0_0 * ui0_1;
                a11 += ui0_1 * ui0_1;

                float ui1_0 = U[i + 1][0];
                float ui1_1 = U[i + 1][1];
                a00 += ui1_0 * ui1_0;
                a01 += ui1_0 * ui1_1;
                a11 += ui1_1 * ui1_1;

                b0 += ui0_0 * rij0 + ui1_0 * rij1;
                b1 += ui0_1 * rij0 + ui1_1 * rij1;
            }
            for (; i < ROWS; ++i) {
                float rij = R[i][j];
                float ui0 = U[i][0];
                float ui1 = U[i][1];
                a00 += ui0 * ui0;
                a01 += ui0 * ui1;
                a11 += ui1 * ui1;
                b0 += ui0 * rij;
                b1 += ui1 * rij;
            }

            float a10 = a01;
            float det = a00 * a11 - a01 * a10;
            if (det == 0.0f) det = 1e-6f;
            float inv00 =  a11 / det;
            float inv01 = -a01 / det;
            float inv10 = -a10 / det;
            float inv11 =  a00 / det;

            float new0 = inv00 * b0 + inv01 * b1;
            float new1 = inv10 * b0 + inv11 * b1;
            V[j][0] = new0;
            V[j][1] = new1;
        }
    }

    // ----- output result matrices -----
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "User factors (U):\n";
    for (int i = 0; i < ROWS; ++i) {
        for (int f = 0; f < FACT; ++f)
            std::cout << U[i][f] << ' ';
        std::cout << '\n';
    }
    std::cout << "\nItem factors (V):\n";
    for (int j = 0; j < COLS; ++j) {
        for (int f = 0; f < FACT; ++f)
            std::cout << V[j][f] << ' ';
        std::cout << '\n';
    }

    // optional: reconstruct approximation and show error
    std::cout << "\nReconstructed ratings (U*V^T):\n";
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            float est = U[i][0] * V[j][0] + U[i][1] * V[j][1];
            std::cout << est << ' ';
        }
        std::cout << '\n';
    }
    return 0;
}
