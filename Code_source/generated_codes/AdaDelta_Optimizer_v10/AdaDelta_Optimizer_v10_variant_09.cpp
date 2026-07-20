/* LLM input variant 9: medium-deterministic-random */
// AdaDelta Optimizer – version #10
// Everything is inside main(), using only float/int, std::vector and manual loop unrolling.

#include <iostream>
#include <vector>
#include <cmath>

int main() {
    // Hyper‑parameters (float, no const)
    float rho = 0.95f;          // decay factor
    float eps = 1e-6f;          // stability term
    int   steps = 5;            // number of optimization steps

    // Deterministic test vectors (size = 8, pseudo‑random looking)
    std::vector<float> param = { 1.0f, -2.5f, 0.75f, 3.3f, -1.2f, 2.8f, -0.6f, 4.1f };
    std::vector<float> eg2   (param.size(), 0.0f); // running avg of grad^2
    std::vector<float> edx2  (param.size(), 0.0f); // running avg of delta^2

    // Optimization loop
    for (int it = 0; it < steps; ++it) {
        // ---- compute gradients (quadratic loss L = sum w_i^2) ----
        std::vector<float> grad(param.size());
        for (size_t i = 0; i < param.size(); ++i) {
            grad[i] = 2.0f * param[i];
        }

        // ---- update running average of squared gradients ----
        for (size_t i = 0; i < param.size(); ++i) {
            float tmp = grad[i] * grad[i];
            eg2[i] = rho * eg2[i] + (1.0f - rho) * tmp;
        }

        // ---- compute RMS values and parameter updates ----
        for (size_t i = 0; i < param.size(); ++i) {
            float rmsdx = std::sqrt(edx2[i] + eps);
            float rmsg  = std::sqrt(eg2[i] + eps);
            float upd   = - (rmsdx / rmsg) * grad[i];
            param[i]   += upd;

            // ---- update running average of squared updates ----
            float upd_sq = upd * upd;
            edx2[i] = rho * edx2[i] + (1.0f - rho) * upd_sq;
        }
    }

    // Print final parameters
    std::cout << "Final parameters after AdaDelta:" << std::endl;
    for (size_t i = 0; i < param.size(); ++i) {
        std::cout << "w[" << i << "] = " << param[i] << std::endl;
    }

    return 0;
}
