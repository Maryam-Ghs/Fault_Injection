#include <iostream>
#include <cstdlib>
#include <ctime>
#include <iomanip>

/* LLM input variant 8: sparse-skewed */

#define ROWS 5       // number of users
#define COLS 6       // number of items
#define FACT 2       // latent factors
#define ITER 8       // ALS iterations
#define LAMB 0.1f    // regularization

int main() {
    // seed random generator (kept for reproducibility of factor init)
    std::srand((unsigned)std::time(0));

    // ----- generate sparse, skewed rating matrix (ROW x COL) -----
    float R[ROWS][COLS];
    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c < COLS; ++c) {
            // Create a highly sparse and imbalanced pattern:
            // - First user (row 0) has a few high ratings clustered at columns 0 and 3.
            // - Last user (row ROWS-1) has a single rating at the last column.
            // - Other users have sporadic low ratings following a simple modulus rule.
            if (r == 0) {
                R[r][c] = (c == 0 || c == 3) ? 5.0f : 0.0f;
            } else if (r == ROWS - 1) {
                R[r][c] = (c == COLS - 1) ? 4.0f : 0.0f;
            } else {
                R[r][c] = ((r + c) % 7 == 0) ? 3.0f : 0.0f;
            }
        }
    }

    // ----- factor matrices: USER (ROW x FACT) & ITEM (COL x FACT) -----
    float U[ROWS][FACT];
    float V[COLS][FACT];
    for (int i = 0; i < ROWS; ++i)
        for (int f = 0; f < FACT; ++f)
            U[i][f] = 0.1f * (float)(std::rand() % 10 + 1);
    for (int j = 0; j < COLS; ++j)
        for (int f = 0; f < FACT; ++f)
            V[j][f] = 0.1f * (float)(std::rand() % 10 + 1);

    // ----- ALS core -----
    for (int it = 0; it < ITER; ++it) {
        // ---- update USER factors (U) ----
        for (int i = 0; i < ROWS; ++i) {
            // compute A = V^T * V + LAMB * I   (2x2 matrix)
            float a00 = LAMB, a01 = 0.0f, a11 = LAMB;
            // compute b = V^T * r_i   (2-dimensional vector)
            float b0 = 0.0f, b1 = 0.0f;

            // manual unroll over items (process two at a time)
            int j = 0;
            for (; j <= COLS - 2; j += 2) {
                float rj0 = R[i][j];
                float rj1 = R[i][j + 1];

                // accumulate A
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

                // accumulate b (reordered: multiply then add)
                b0 += vj0_0 * rj0 + vj1_0 * rj1;
                b1 += vj0_1 * rj0 + vj1_1 * rj1;
            }
            // tail element if COLS is odd
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

            // make A symmetric
            float a10 = a01;

            // solve 2x2 linear system A * x = b
            float det = a00 * a11 - a01 * a10;
            // guard against zero determinant (add tiny jitter)
            if (det == 0.0f) det = 1e-6f;
            float inv00 =  a11 / det;
            float inv01 = -a01 / det;
            float inv10 = -a10 / det;
            float inv11 =  a00 / det;

            // compute new user factors (reordered multiplication)
            float new0 = inv00 * b0 + inv01 * b1;
            float new1 = inv10 * b0 + inv11 * b1;
            U[i][0] = new0;
            U[i][1] = new1;
        }

        // ---- update ITEM factors (V) ----
        for (int j = 0; j < COLS; ++j) {
            // compute A = U^T * U + LAMB * I   (2x2)
            float a00 = LAMB, a01 = 0.0f, a11 = LAMB;
            // compute b = U^T * r_·j   (2-dim)
            float b0 = 0.0f, b1 = 0.0f;

            // manual unroll over users
            int i = 0;
            for (; i <= ROWS - 2; i += 2) {
                float rij0 = R[i][j];
                float rij1 = R[i + 1][j];

                // accumulate A
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

                // accumulate b (reordered)
                b0 += ui0_0 * rij0 + ui1_0 * rij1;
                b1 += ui0_1 * rij0 + ui1_1 * rij1;
            }
            // tail element if ROWS is odd
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

            // symmetric A
            float a10 = a01;

            // solve 2x2 system
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
