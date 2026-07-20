#include <iostream>
#include <cmath>

/* LLM input variant 4: signed-extremes */

int main() {
    // hyper‑parameters (float only)
    float rho = 0.90f;          // decay factor
    float eps = 1e-6f;          // epsilon to avoid division by zero

    // deterministic test vectors – three parameters with mixed signs and zero
    float w[3] = { -1.0e3f, 0.0f, 1.0e3f };
    // accumulators for squared gradients and squared updates
    float acc_g[3] = { 0.0f, 0.0f, 0.0f };
    float acc_u[3] = { 0.0f, 0.0f, 0.0f };

    // number of optimisation steps
    int steps = 12;

    // iterative loop (outer = steps, inner = parameters)
    for (int s = 0; s < steps; ++s) {
        // compute gradient of f(w)=0.5*w^2 => grad = w
        for (int i = 0; i < 3; ++i) {
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
    for (int i = 0; i < 3; ++i) {
        std::cout << "w[" << i << "] = " << w[i] << '\n';
    }
    return 0;
}
