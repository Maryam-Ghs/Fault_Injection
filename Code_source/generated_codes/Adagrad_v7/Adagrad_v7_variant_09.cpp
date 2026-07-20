/*
 * LLM input variant 9: medium-deterministic-random
 *
 * Adagrad optimizer – version #7
 * -------------------------------------------------
 * Implements a tiny deterministic linear regression
 * using Adagrad with manual loop unrolling, reordered
 * arithmetic, stack‑allocated arrays and helper functions.
 *
 * Constraints:
 *   – only float / int (no double, long, unsigned, const)
 *   – no cin, input is generated inside the program
 *   – results are printed to stdout
 */

#include <iostream>
#include <cmath>

#define LR   0.05f         // base learning rate (variant)
#define EPS  1e-8f
#define DIM  3
#define BATCH 4
#define STEPS 6            // increased steps for richer evolution

/* ----------------------------------------------------------------- */
/* Helper: initialise weight vector and accumulated squared grads      */
void init_state(float *w, float *acc) {
    int i = 0;
    while (i < DIM) {
        w[i]   = 0.0f;
        acc[i] = 0.0f;
        ++i;
    }
}

/* ----------------------------------------------------------------- */
/* Helper: compute gradient of the squared loss for a single sample   */
void grad_one(const float *w, const float *x, float y, float *g) {
    /* prediction = w·x  (manual unroll) */
    float pred = w[0] * x[0] + w[1] * x[1] + w[2] * x[2];
    /* error = pred - y */
    float err  = pred - y;

    /* gradient = 2 * error * x   (reordered arithmetic) */
    g[0] = (err + err) * x[0];
    g[1] = (err + err) * x[1];
    g[2] = (err + err) * x[2];
}

/* ----------------------------------------------------------------- */
/* Helper: accumulate gradient over the whole batch                    */
void batch_gradient(const float *w,
                    const float X[BATCH][DIM],
                    const float Y[BATCH],
                    float *g_tot) {
    int j = 0;
    while (j < DIM) {
        g_tot[j] = 0.0f;
        ++j;
    }

    int s = 0;
    while (s < BATCH) {
        float g_one[DIM];
        grad_one(w, X[s], Y[s], g_one);

        g_tot[0] += g_one[0];
        g_tot[1] += g_one[1];
        g_tot[2] += g_one[2];
        ++s;
    }

    int k = 0;
    while (k < DIM) {
        g_tot[k] = g_tot[k] / static_cast<float>(BATCH);
        ++k;
    }
}

/* ----------------------------------------------------------------- */
/* Helper: perform a single Adagrad update step                        */
void adagrad_update(float *w, float *acc, const float *g) {
    int i = 0;
    while (i < DIM) {
        acc[i] = acc[i] + g[i] * g[i];
        float adj = LR / std::sqrt(acc[i] + EPS);
        w[i] = w[i] - adj * g[i];
        ++i;
    }
}

/* ----------------------------------------------------------------- */
/* Helper: print current parameters                                    */
void show_weights(const float *w) {
    std::cout << "w = [ ";
    int i = 0;
    while (i < DIM) {
        std::cout << w[i];
        if (i < DIM - 1) std::cout << ", ";
        ++i;
    }
    std::cout << " ]\n";
}

/* ----------------------------------------------------------------- */
int main() {
    /* deterministic pseudo‑random‑looking training data */
    float X[BATCH][DIM] = {
        { 0.5f, -1.0f,  2.5f},
        { 3.3f, -0.7f,  4.1f},
        {-2.0f,  1.5f, -3.3f},
        { 5.0f,  0.0f, -1.2f}
    };
    float Y[BATCH] = { -0.5f, 1.8f, -2.2f, 0.0f };

    float w[DIM];
    float acc_sq[DIM];

    init_state(w, acc_sq);

    std::cout << "Adagrad optimisation (Version #7 - Variant 9)\n";

    int step = 0;
    while (step < STEPS) {
        float grad[DIM];
        batch_gradient(w, X, Y, grad);
        adagrad_update(w, acc_sq, grad);

        std::cout << "Step " << step + 1 << ": ";
        show_weights(w);
        ++step;
    }

    return 0;
}
