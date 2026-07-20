#include <cstdio>

/* LLM input variant 5: duplicate-heavy */

int main() {
    /* ---------- affine matrix (row‑major) ---------- */
    float* M = new float[16];
    /* duplicate-heavy matrix: all entries set to 1.0 (except last stays 1) */
    for (int i = 0; i < 16; ++i) {
        M[i] = 1.0f;
    }
    M[15] = 1.0f;  // homogeneous coordinate row

    /* ---------- deterministic test points ---------- */
    int N = 8;                       // number of points (duplicate-heavy)
    float* P = new float[N * 4];    // homogeneous coordinates

    /* all points are identical: (2, 2, 2, 1) */
    for (int i = 0; i < N; ++i) {
        int base = i * 4;
        P[base] = 2.0f;      // x
        P[base + 1] = 2.0f;  // y
        P[base + 2] = 2.0f;  // z
        P[base + 3] = 1.0f;  // w
    }

    /* ---------- result buffer ---------- */
    float* R = new float[N * 4];

    /* ---------- manual loop unrolling, expanded ops ---------- */
    for (int idx = 0; idx < N; ++idx) {
        int base = idx * 4;
        float x = P[base];
        float y = P[base + 1];
        float z = P[base + 2];

        /* compute X' */
        float a0 = M[0] * x;
        float a1 = M[1] * y;
        float a2 = M[2] * z;
        float Xp = a0 + a1;
        Xp = Xp + a2 + M[3];

        /* compute Y' */
        float b0 = M[4] * x;
        float b1 = M[5] * y;
        float b2 = M[6] * z;
        float Yp = b0 + b1;
        Yp = Yp + b2 + M[7];

        /* compute Z' */
        float c0 = M[8] * x;
        float c1 = M[9] * y;
        float c2 = M[10] * z;
        float Zp = c0 + c1;
        Zp = Zp + c2 + M[11];

        /* compute W (should stay 1) */
        float Wp = M[12] * x + M[13] * y + M[14] * z + M[15];

        /* store */
        R[base]     = Xp;
        R[base + 1] = Yp;
        R[base + 2] = Zp;
        R[base + 3] = Wp;
    }

    /* ---------- output ---------- */
    for (int i = 0; i < N; ++i) {
        int b = i * 4;
        printf("Point %d transformed: (%.3f, %.3f, %.3f, %.3f)\n",
               i, R[b], R[b + 1], R[b + 2], R[b + 3]);
    }

    delete[] M;
    delete[] P;
    delete[] R;
    return 0;
}
