#include <iostream>
#include <cmath>

/* LLM input variant 1: minimal-boundary */

int main() {
    // ---------- hyper‑parameters ----------
    float lr   = 0.002f;      // learning rate
    float b1   = 0.9f;        // beta1
    float b2   = 0.999f;      // beta2
    float eps  = 1e-8f;       // epsilon for numerical stability

    // ---------- problem set‑up ----------
    const int dim = 1;                     // minimal number of parameters
    const int steps = 1;                  // minimal number of gradient steps
    float theta[dim] = { 5.0f };           // initial parameter
    float m[dim] = { 0.0f };              // first moment estimate
    float u[dim] = { 0.0f };              // infinity norm estimate

    // ---------- artificially minimal gradient sequence ----------
    float grads[steps][dim] = {
        { 1.0f }                         // smallest non‑zero gradient
    };

    // ---------- optimizer loop ----------
    float b1_pow = 1.0f;   // will hold b1^t
    for (int step = 1; step <= steps; ++step) {
        // update bias‑correction term
        b1_pow = b1_pow * b1;                     // b1_pow = b1^step

        // ---- element 0 ----
        float g0 = grads[step-1][0];
        float abs_g0 = (g0 >= 0.0f) ? g0 : -g0;
        m[0] = b1 * m[0] + (1.0f - b1) * g0;       // first moment
        float ub0 = b2 * u[0];
        u[0] = (ub0 > abs_g0) ? ub0 : abs_g0;     // infinity norm
        float lr_corr0 = lr / (1.0f - b1_pow);    // bias‑corrected lr
        float delta0 = lr_corr0 * (m[0] / (u[0] + eps));
        theta[0] = theta[0] - delta0;
    }

    // ---------- output ----------
    std::cout << "Final parameters after AdaMax (version #5):\n";
    for (int i = 0; i < dim; ++i) {
        std::cout << "theta[" << i << "] = " << theta[i] << "\n";
    }
    return 0;
}
