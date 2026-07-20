/* LLM input variant 5: duplicate-heavy */
#include <iostream>
#include <cmath>

#define DIM 12          // dimension of the parameter vector
#define STEPS 7         // number of AdaDelta updates

int main() {
    // ---------- deterministic initial parameters and gradients ----------
    float param[DIM];
    float grad[DIM];
    // Use duplicate-heavy values: all parameters start at 0.5, all gradients at 0.1
    for (int i = 0; i < DIM; ++i) {
        param[i] = 0.5f;
        grad[i]  = 0.1f;
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
        // deterministic repeated gradient pattern for this step:
        // even indices get 0.1, odd indices get -0.1 (many duplicates)
        for (int i = 0; i < DIM; ++i) {
            grad[i] = (i % 2 == 0) ? 0.1f : -0.1f;
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
