#include <iostream>
#include <cmath>

/* LLM input variant 7: reverse-adversarial */

int main() {
    // hyper‑parameters (float only)
    float rho = 0.0f;           // decay factor set to zero for immediate averaging (adversarial)
    float eps = 1e-12f;         // very small epsilon to keep division safe

    // deterministic test vectors – three parameters (reverse‑ordered, extreme magnitudes)
    float w[3] = { 1e-6f, -1e6f, 1e6f };
    // accumulators for squared gradients and squared updates
    float acc_g[3] = { 0.0f, 0.0f, 0.0f };
    float acc_u[3] = { 0.0f, 0.0f, 0.0f };

    // number of optimisation steps (increased for stress)
    int steps = 20;

    // iterative loop (outer = steps, inner = parameters)
    for (int s = 0; s < steps; ++s) {
        // compute gradient of f(w)=0.5*w^2 => grad = w
        for (int i = 0; i < 3; ++i) {
            // square of gradient, but reorder: multiply then add
            float g = w[i];
            float g2 = g * g;                     // g²
            // exponential moving average of squared gradients (rho = 0 => just current)
            acc_g[i] = (1.0f - rho) * g2;

            // compute RMS of past updates and current gradients (reordered)
            float rms_u = std::sqrt(acc_u[i] + eps);
            float rms_g = std::sqrt(acc_g[i] + eps);

            // delta = - (rms_u / rms_g) * g  (order changed)
            float factor = rms_u / rms_g;
            float delta = - factor * g;

            // update accumulator for squared updates
            float d2 = delta * delta;               // Δ²
            acc_u[i] = (1.0f - rho) * d2;

            // apply update
            w[i] = w[i] + delta;
        }
    }

    // output final parameters
    std::cout << "Final parameters after AdaDelta:\n";
    for (int i = 0; i < 3; ++i) {
        std::cout << "w[" << i << "] = " << w[i] << '\n';
    }
    return 0;
}
