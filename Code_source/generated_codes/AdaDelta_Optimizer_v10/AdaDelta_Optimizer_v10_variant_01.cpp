/ * LLM input variant 1: minimal-boundary * /
#include <iostream>
#include <vector>
#include <cmath>

int main() {
    // Hyper‑parameters (float, no const)
    float rho = 0.95f;          // decay factor
    float eps = 1e-6f;          // stability term
    int   steps = 1;            // minimal non‑trivial number of steps

    // Deterministic test vectors (size = 1)
    std::vector<float> param = { 1.0f };
    std::vector<float> eg2   = { 0.0f };
    std::vector<float> edx2  = { 0.0f };

    // Optimization loop (single‑parameter, unrolled)
    for (int it = 0; it < steps; ++it) {
        // ---- compute gradient (quadratic loss L = w^2) ----
        float g0 = 2.0f * param[0];

        // ---- update running average of squared gradients ----
        float tmp0 = g0 * g0;
        eg2[0] = rho * eg2[0] + (1.0f - rho) * tmp0;

        // ---- compute RMS values ----
        float rmsdx0 = std::sqrt(edx2[0] + eps);
        float rmsg0  = std::sqrt(eg2[0]  + eps);

        // ---- compute parameter update ----
        float upd0 = - (rmsdx0 / rmsg0) * g0;

        // ---- apply update ----
        param[0] += upd0;

        // ---- update running average of squared updates ----
        float upd_sq0 = upd0 * upd0;
        edx2[0] = rho * edx2[0] + (1.0f - rho) * upd_sq0;
    }

    // Print final parameter
    std::cout << "Final parameters after AdaDelta:" << std::endl;
    std::cout << "w[0] = " << param[0] << std::endl;

    return 0;
}
