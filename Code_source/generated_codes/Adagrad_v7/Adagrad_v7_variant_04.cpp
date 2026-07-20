/*
 * LLM input variant 4: signed-extremes
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

#define LR   0.1f          // base learning rate
#define EPS  1e-8f         // small number to avoid division by zero
#define DIM  3             // dimensionality of the weight vector
#define BATCH 4            // number of training samples
#define STEPS 5            // optimisation steps

/* ----------------------------------------------------------------- */
/* Helper: initialise weight vector and accumulated squared grads      */
void init_state(float *w, float *acc) {
    int i = 0;
    while (i < DIM) {
        w[i]   = 0.0f;      // start from zero
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
    /* zero the accumulator */
    int j = 0;
    while (j < DIM) {
        g_tot[j] = 0.0f;
        ++j;
    }

    /* iterate over samples (while‑loop to vary structure) */
    int s = 0;
    while (s < BATCH) {
        float g_one[DIM];
        grad_one(w, X[s], Y[s], g_one);

        /* accumulate (manual unroll) */
        g_tot[0] += g_one[0];
        g_tot[1] += g_one[1];
        g_tot[2] += g_one[2];
        ++s;
    }

    /* average gradient */
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
        /* accumulate squared gradients first (reordered) */
        acc[i] = acc[i] + g[i] * g[i];

        /* adjusted learning rate */
        float adj = LR / std::sqrt(acc[i] + EPS);

        /* weight update (manual unroll) */
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
    /* deterministic training data (stack arrays) */
    float X[BATCH][DIM] = {
        { -5.0f,  0.0f,  3.0f},
        {  2.0f, -4.0f,  0.0f},
        {  0.0f,  5.0f, -1.0f},
        { -2.5f,  1.5f, -3.5f}
    };
    float Y[BATCH] = { -1.0f, 0.0f, 2.0f, -0.5f };

    /* optimizer state */
    float w[DIM];
    float acc_sq[DIM];

    init_state(w, acc_sq);

    std::cout << "Adagrad optimisation (Version #7)\n";

    /* optimisation loop */
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
