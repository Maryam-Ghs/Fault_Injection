#include <iostream>
#include <cmath>

/* LLM input variant 10: large-safe-stress */

int main() {
    // ---------- hyper‑parameters ----------
    float lr   = 0.002f;      // learning rate
    float b1   = 0.9f;        // beta1
    float b2   = 0.999f;      // beta2
    float eps  = 1e-8f;       // epsilon for numerical stability

    // ---------- problem set‑up ----------
    const int dim   = 100;    // increased number of parameters
    const int steps = 200;   // increased number of optimization steps

    float theta[dim];
    float m[dim];
    float u[dim];

    // deterministic initialization
    for (int i = 0; i < dim; ++i) {
        theta[i] = 5.0f - 0.05f * i;   // varied initial parameters
        m[i]     = 0.0f;
        u[i]     = 0.0f;
    }

    // ---------- optimizer loop ----------
    float b1_pow = 1.0f;   // will hold b1^t

    for (int step = 1; step <= steps; ++step) {
        // update bias‑correction term
        b1_pow *= b1;                     // b1_pow = b1^step
        float lr_corr = lr / (1.0f - b1_pow); // bias‑corrected lr (same for all dims this step)

        for (int i = 0; i < dim; ++i) {
            // deterministic gradient generation
            // Mix of extreme values, small values, and a periodic component
            float base   = (step % 2 == 0) ? 1e6f : -1e6f;
            float scale  = (i % 3 == 0) ? 1.0f : 0.5f;
            float offset = static_cast<float>(step % 7);
            float g = base * scale + offset;

            float abs_g = (g >= 0.0f) ? g : -g;
            m[i] = b1 * m[i] + (1.0f - b1) * g;          // first moment estimate
            float ub = b2 * u[i];
            u[i] = (ub > abs_g) ? ub : abs_g;           // infinity norm estimate
            float delta = lr_corr * (m[i] / (u[i] + eps));
            theta[i] = theta[i] - delta;
        }
    }

    // ---------- output ----------
    std::cout << "Final parameters after AdaMax (version #5) - large-safe-stress variant:\n";
    for (int i = 0; i < dim; ++i) {
        std::cout << "theta[" << i << "] = " << theta[i] << "\n";
    }
    return 0;
}
