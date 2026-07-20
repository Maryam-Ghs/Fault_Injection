#include <iostream>
#include <cmath>

/* LLM input variant 8: sparse-skewed */

#define DIM 12          // dimension of the parameter vector
#define STEPS 7         // number of AdaDelta updates

int main() {
    // ---------- deterministic sparse initial parameters ----------
    float param[DIM];
    for (int i = 0; i < DIM; ++i) {
        // Sparse: only every 4th element gets a moderate non‑zero value
        param[i] = (i % 4 == 0) ? 0.5f : 0.0f;
    }

    // ---------- deterministic sparse gradients ----------
    float grad[DIM];
    // Initially zero; will be overwritten each step with a sparse pattern
    for (int i = 0; i < DIM; ++i) grad[i] = 0.0f;

    // ---------- AdaDelta state (accumulated squared grads and deltas) ----------
    float accGrad[DIM];
    float accDelta[DIM];
    for (int i = 0; i < DIM; ++i) {
        accGrad[i]  = 0.0f;
        accDelta[i] = 0.0f;
    }

    // hyper‑parameters
    float rho = 0.95f;
    float eps = 1e-6f;

    // ---------- perform a few optimization steps ----------
    int step = 0;
    while (step < STEPS) {
        // generate a sparse, skewed gradient for this step
        for (int i = 0; i < DIM; ++i) {
            // Only a couple of dimensions receive a sizable gradient; others stay zero
            if (i == 2) grad[i] =  0.8f;   // positive spike
            else if (i == 8) grad[i] = -0.6f; // negative spike
            else grad[i] = 0.0f;
        }

        // update each dimension
        int idx = 0;
        while (idx < DIM) {
            // 1) update running average of squared gradients
            float g2 = grad[idx] * grad[idx];
            float tmp1 = rho * accGrad[idx];
            float tmp2 = (1.0f - rho) * g2;
            accGrad[idx] = tmp1 + tmp2;

            // 2) compute RMS of accumulated grads
            float rmsGrad = std::sqrt(accGrad[idx] + eps);

            // 3) compute RMS of accumulated deltas (from previous step)
            float rmsDelta = std::sqrt(accDelta[idx] + eps);

            // 4) compute adaptive learning rate factor
            float factor = rmsDelta / rmsGrad;

            // 5) compute parameter update (delta)
            float delta = -factor * grad[idx];

            // 6) apply update
            param[idx] = param[idx] + delta;

            // 7) update running average of squared deltas
            float d2 = delta * delta;
            float tmp3 = rho * accDelta[idx];
            float tmp4 = (1.0f - rho) * d2;
            accDelta[idx] = tmp3 + tmp4;

            ++idx; // advance to next dimension
        }

        ++step; // next optimization step
    }

    // ---------- output final parameters ----------
    std::cout << "AdaDelta final parameters (version #4):\n";
    for (int i = 0; i < DIM; ++i) {
        std::cout << param[i] << (i + 1 == DIM ? '\n' : ' ');
    }
    return 0;
}
