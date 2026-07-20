#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>

/* LLM input variant 6: ordered-structured */

#define DIM 12          // dimension of the parameter vector
#define STEPS 7         // number of AdaDelta updates

int main() {
    // seed random generator (kept for compatibility, not used)
    std::srand((unsigned)std::time(0));

    // ---------- generate deterministic initial parameters and gradients ----------
    float param[DIM];
    float grad[DIM];
    // Ordered, symmetric initialization: values increase linearly from -0.55 to 0.55
    for (int i = 0; i < DIM; ++i) {
        param[i] = (i - DIM / 2) * 0.1f;   // e.g., -0.6, -0.5, ..., 0.5
        grad[i]  = (i % 2 == 0 ? 0.05f : -0.05f); // alternating small gradient
    }

    // ---------- AdaDelta state (accumulated squared grads and deltas) ----------
    float accGrad[DIM];
    float accDelta[DIM];
    for (int i = 0; i < DIM; ++i) {
        accGrad[i]  = 0.0f;
        accDelta[i] = 0.0f;
    }

    // hyper‑parameters (no const per the constraints)
    float rho = 0.95f;
    float eps = 1e-6f;

    // ---------- perform a few optimization steps ----------
    int step = 0;
    while (step < STEPS) {
        // deterministic gradient pattern for this step:
        // shift the alternating pattern by step positions to keep it structured
        for (int i = 0; i < DIM; ++i) {
            grad[i] = ((i + step) % 2 == 0 ? 0.05f : -0.05f);
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

            ++idx;                               // advance to next dimension
        }

        ++step;                                   // next optimization step
    }

    // ---------- output final parameters ----------
    std::cout << "AdaDelta final parameters (version #4):\n";
    for (int i = 0; i < DIM; ++i) {
        std::cout << param[i] << (i + 1 == DIM ? '\n' : ' ');
    }
    return 0;
}
