#include <cstdio>

/* LLM input variant 9: medium-deterministic-random */

int main() {
    /* ---------- affine matrix (row‑major) ---------- */
    float* M = new float[16];
    /* scaling 1.5, rotation Y 30°, translation (-2, 4, -1) */
    float c = 0.8660254f;   // cos(30°)
    float s = 0.5f;         // sin(30°)

    M[0] = 1.5f * c;   M[1] = 0.0f;      M[2] = 1.5f * s;   M[3] = -2.0f;
    M[4] = 0.0f;       M[5] = 1.5f;      M[6] = 0.0f;        M[7] =  4.0f;
    M[8] = -1.5f * s;  M[9] = 0.0f;      M[10]= 1.5f * c;   M[11]= -1.0f;
    M[12]= 0.0f;       M[13]= 0.0f;      M[14]= 0.0f;        M[15]= 1.0f;

    /* ---------- deterministic test points ---------- */
    int N = 7;                       // number of points
    float* P = new float[N * 4];    // homogeneous coordinates

    /* point 0 : (-1, -2, -2, 1) */
    P[0] = -1.0f; P[1] = -2.0f; P[2] = -2.0f; P[3] = 1.0f;
    /* point 1 : (0, -1, -1, 1) */
    P[4] = 0.0f;  P[5] = -1.0f; P[6] = -1.0f; P[7] = 1.0f;
    /* point 2 : (1, 0, 0, 1) */
    P[8] = 1.0f;  P[9] = 0.0f;  P[10]= 0.0f;  P[11]= 1.0f;
    /* point 3 : (-1, 1, 1, 1) */
    P[12]= -1.0f; P[13]= 1.0f; P[14]= 1.0f; P[15]= 1.0f;
    /* point 4 : (0, -2, -2, 1) */
    P[16]= 0.0f;  P[17]= -2.0f; P[18]= -2.0f; P[19]= 1.0f;
    /* point 5 : (1, -1, -1, 1) */
    P[20]= 1.0f;  P[21]= -1.0f; P[22]= -1.0f; P[23]= 1.0f;
    /* point 6 : (-1, 0, 0, 1) */
    P[24]= -1.0f; P[25]= 0.0f;  P[26]= 0.0f;  P[27]= 1.0f;

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
