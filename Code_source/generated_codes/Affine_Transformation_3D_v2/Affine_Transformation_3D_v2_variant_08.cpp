#include <cstdio>

/* LLM input variant 8: sparse-skewed */
int main() {
    /* ---------- affine matrix (row‑major) ---------- */
    float* M = new float[16];
    /* scaling 2, rotation Z 45°, translation (5, -3, 2) */
    float c = 0.70710678f;   // cos(45°)
    float s = 0.70710678f;   // sin(45°)

    M[0] = 2 * c;  M[1] = -2 * s; M[2] = 0;   M[3] = 5;
    M[4] = 2 * s;  M[5] =  2 * c; M[6] = 0;   M[7] = -3;
    M[8] = 0;      M[9] = 0;      M[10]= 0;  M[11]= 0;   // Z‑axis scaling zeroed for sparsity
    M[12]= 0;      M[13]= 0;      M[14]= 0;  M[15]= 1;

    /* ---------- deterministic test points (sparse, skewed) ---------- */
    int N = 12;                       // number of points
    float* P = new float[N * 4];    // homogeneous coordinates

    // Clustered non‑zero points
    // point 0 : (1,0,0,1)
    P[0] = 1; P[1] = 0; P[2] = 0; P[3] = 1;
    // point 1 : (0,1,0,1)
    P[4] = 0; P[5] = 1; P[6] = 0; P[7] = 1;
    // point 2 : (0,0,1,1) – will be collapsed by zero Z‑scale
    P[8] = 0; P[9] = 0; P[10]= 1; P[11]= 1;
    // point 3 : (2,2,0,1)
    P[12]= 2; P[13]= 2; P[14]= 0; P[15]= 1;
    // point 4 : (-1, -1, 0, 1)
    P[16]= -1; P[17]= -1; P[18]= 0; P[19]= 1;

    // Remaining points are all zeros (sparse)
    for (int i = 5; i < N; ++i) {
        int b = i * 4;
        P[b] = 0; P[b+1] = 0; P[b+2] = 0; P[b+3] = 1;
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
