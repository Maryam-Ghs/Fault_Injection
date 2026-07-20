#include <cstdio>
#include <cstdlib>
#include <ctime>

/* LLM input variant 1: minimal-boundary */
int main() {
    /* dimensions and parameters */
    int ucnt = 1;          // number of users
    int icnt = 1;          // number of items
    int k    = 1;          // latent factors
    int iters = 1;         // ALS iterations
    float lam = 0.1f;      // regularisation

    srand(1);

    /* rating matrix R (users × items) */
    float *R = new float[ucnt * icnt];
    for (int p = 0; p < ucnt * icnt; ++p)
        R[p] = (float)(rand() % 5);               // random integer rating 0‑4

    /* factor matrices U (users × k) and V (items × k) */
    float *U = new float[ucnt * k];
    float *V = new float[icnt * k];
    for (int p = 0; p < ucnt * k; ++p)
        U[p] = (float)rand() / RAND_MAX;          // small random numbers
    for (int p = 0; p < icnt * k; ++p)
        V[p] = (float)rand() / RAND_MAX;

    /* ALS loop */
    for (int it = 0; it < iters; ++it) {
        /* ---- compute Vᵀ·V --------------------------------------------------- */
        float *VtV = new float[k * k];
        for (int a = 0; a < k * k; ++a) VtV[a] = 0.0f;
        for (int i = 0; i < icnt; ++i) {
            for (int a = 0; a < k; ++a) {
                float vai = V[i * k + a];
                for (int b = 0; b < k; ++b)
                    VtV[a * k + b] += vai * V[i * k + b];
            }
        }
        for (int a = 0; a < k; ++a) VtV[a * k + a] += lam;

        /* ---- invert Vᵀ·V (Gaussian elimination) --------------------------- */
        float *invV = new float[k * k];
        for (int a = 0; a < k; ++a)
            for (int b = 0; b < k; ++b)
                invV[a * k + b] = (a == b) ? 1.0f : 0.0f;

        float *M = new float[k * k];
        for (int a = 0; a < k * k; ++a) M[a] = VtV[a];

        for (int p = 0; p < k; ++p) {
            float diag = M[p * k + p];
            for (int q = 0; q < k; ++q) {
                M[p * k + q] /= diag;
                invV[p * k + q] /= diag;
            }
            for (int r = 0; r < k; ++r) if (r != p) {
                float factor = M[r * k + p];
                for (int c = 0; c < k; ++c) {
                    M[r * k + c] -= factor * M[p * k + c];
                    invV[r * k + c] -= factor * invV[p * k + c];
                }
            }
        }
        delete[] M;
        delete[] VtV;

        /* ---- update user factors U ---------------------------------------- */
        for (int u = 0; u < ucnt; ++u) {
            float *VtR = new float[k];
            for (int a = 0; a < k; ++a) VtR[a] = 0.0f;

            for (int i = 0; i < icnt; ++i) {
                float rui = R[u * icnt + i];
                for (int a = 0; a < k; ++a)
                    VtR[a] += V[i * k + a] * rui;
            }

            for (int a = 0; a < k; ++a) {
                float sum = 0.0f;
                for (int b = 0; b < k; ++b)
                    sum += invV[a * k + b] * VtR[b];
                U[u * k + a] = sum;
            }
            delete[] VtR;
        }
        delete[] invV;

        /* ---- compute Uᵀ·U --------------------------------------------------- */
        float *UtU = new float[k * k];
        for (int a = 0; a < k * k; ++a) UtU[a] = 0.0f;
        for (int u = 0; u < ucnt; ++u) {
            for (int a = 0; a < k; ++a) {
                float uua = U[u * k + a];
                for (int b = 0; b < k; ++b)
                    UtU[a * k + b] += uua * U[u * k + b];
            }
        }
        for (int a = 0; a < k; ++a) UtU[a * k + a] += lam;

        /* ---- invert Uᵀ·U --------------------------------------------------- */
        float *invU = new float[k * k];
        for (int a = 0; a < k; ++a)
            for (int b = 0; b < k; ++b)
                invU[a * k + b] = (a == b) ? 1.0f : 0.0f;

        float *N = new float[k * k];
        for (int a = 0; a < k * k; ++a) N[a] = UtU[a];

        for (int p = 0; p < k; ++p) {
            float diag = N[p * k + p];
            for (int q = 0; q < k; ++q) {
                N[p * k + q] /= diag;
                invU[p * k + q] /= diag;
            }
            for (int r = 0; r < k; ++r) if (r != p) {
                float factor = N[r * k + p];
                for (int c = 0; c < k; ++c) {
                    N[r * k + c] -= factor * N[p * k + c];
                    invU[r * k + c] -= factor * invU[p * k + c];
                }
            }
        }
        delete[] N;
        delete[] UtU;

        /* ---- update item factors V ---------------------------------------- */
        for (int i = 0; i < icnt; ++i) {
            float *UtR = new float[k];
            for (int a = 0; a < k; ++a) UtR[a] = 0.0f;

            for (int u = 0; u < ucnt; ++u) {
                float rui = R[u * icnt + i];
                for (int a = 0; a < k; ++a)
                    UtR[a] += U[u * k + a] * rui;
            }

            for (int a = 0; a < k; ++a) {
                float sum = 0.0f;
                for (int b = 0; b < k; ++b)
                    sum += invU[a * k + b] * UtR[b];
                V[i * k + a] = sum;
            }
            delete[] UtR;
        }
        delete[] invU;
    }

    /* ---- print resulting factor matrices --------------------------------- */
    printf("User factors (U):\n");
    for (int u = 0; u < ucnt; ++u) {
        for (int a = 0; a < k; ++a)
            printf("%f ", U[u * k + a]);
        printf("\n");
    }
    printf("\nItem factors (V):\n");
    for (int i = 0; i < icnt; ++i) {
        for (int a = 0; a < k; ++a)
            printf("%f ", V[i * k + a]);
        printf("\n");
    }

    delete[] R;
    delete[] U;
    delete[] V;
    return 0;
}
