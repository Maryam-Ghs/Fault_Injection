#include <iostream>
#include <cmath>

/* LLM input variant 1: minimal-boundary */

int main() {
    // hyper‑parameters (float only)
    float rho = 0.95f;          // decay factor
    float eps = 1e-6f;          // epsilon to avoid division by zero

    // deterministic test vectors – single parameter (minimal nontrivial)
    float w[1] = { 1.0f };
    // accumulators for squared gradients and squared updates
    float acc_g[1] = { 0.0f };
    float acc_u[1] = { 0.0f };

    // minimal number of optimisation steps (nontrivial)
    int steps = 1;

    // iterative loop (outer = steps, inner = parameters)
    for (int s = 0; s < steps; ++s) {
        // compute gradient of f(w)=0.5*w^2 => grad = w
        for (int i = 0; i < 1; ++i) {
            // square of gradient, but reorder: multiply then add
            float g = w[i];
            float g2 = g * g;                     // g²
            // exponential moving average of squared gradients
            acc_g[i] = rho * acc_g[i] + (1.0f - rho) * g2;

            // compute RMS of past updates and current gradients (reordered)
            float rms_u = std::sqrt(acc_u[i] + eps);
            float rms_g = std::sqrt(acc_g[i] + eps);

            // delta = - (rms_u / rms_g) * g  (order changed)
            float factor = rms_u / rms_g;
            float delta = - factor * g;

            // update accumulator for squared updates
            float d2 = delta * delta;               // Δ²
            acc_u[i] = rho * acc_u[i] + (1.0f - rho) * d2;

            // apply update
            w[i] = w[i] + delta;
        }
    }

    // output final parameters
    std::cout << "Final parameters after AdaDelta:\n";
    for (int i = 0; i < 1; ++i) {
        std::cout << "w[" << i << "] = " << w[i] << '\n';
    }
    return 0;
}
