/* LLM input variant 1: minimal-boundary */
#include <iostream>
#include <cmath>

#define DIM 1          // minimal non‑zero dimension
#define STEPS 1        // minimal number of AdaDelta updates

int main() {
    // deterministic initial parameters and gradient (boundary values)
    float param[DIM];
    float grad[DIM];
    param[0] = -1.0f;   // lower bound of original random range
    grad[0]  =  1.0f;   // upper bound (approx) of original random range

    // AdaDelta state (accumulated squared grads and deltas)
    float accGrad[DIM];
    float accDelta[DIM];
    for (int i = 0; i < DIM; ++i) {
        accGrad[i]  = 0.0f;
        accDelta[i] = 0.0f;
    }

    // hyper‑parameters
    float rho = 0.95f;
    float eps = 1e-6f;

    // perform a minimal optimization step
    int step = 0;
    while (step < STEPS) {
        // deterministic fresh gradient for this step
        grad[0] = 1.0f;

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

            ++idx;
        }

        ++step;
    }

    // output final parameters
    std::cout << "AdaDelta final parameters (version #4):\n";
    for (int i = 0; i < DIM; ++i) {
        std::cout << param[i] << (i + 1 == DIM ? '\n' : ' ');
    }
    return 0;
}
