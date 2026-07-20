#include <cstdio>

/* LLM input variant 3: zeros-and-ones */
int main() {
    /* ---------- affine matrix (row‑major) ---------- */
    float* M = new float[16];
    /* identity matrix with homogeneous coordinate */
    M[0] = 1;  M[1] = 0; M[2] = 0;  M[3] = 0;
    M[4] = 0;  M[5] = 1; M[6] = 0;  M[7] = 0;
    M[8] = 0;  M[9] = 0; M[10]= 1;  M[11]= 0;
    M[12]= 0;  M[13]= 0; M[14]= 0;  M[15]= 1;

    /* ---------- deterministic test points ---------- */
    int N = 3;                       // number of points
    float* P = new float[N * 4];    // homogeneous coordinates

    /* point 0 : (0,0,0,1) */
    P[0] = 0; P[1] = 0; P[2] = 0; P[3] = 1;
    /* point 1 : (1,1,1,1) */
    P[4] = 1; P[5] = 1; P[6] = 1; P[7] = 1;
    /* point 2 : (1,0,0,1) */
    P[8] = 1; P[9] = 0; P[10]= 0; P[11]= 1;

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
