/* LLM input variant 6: ordered-structured */
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

    // Deterministic test vectors (size = 4) – ordered and symmetric
    std::vector<float> param   = { -3.0f, -1.0f, 1.0f, 3.0f };
    std::vector<float> eg2     = { 0.0f, 0.0f, 0.0f, 0.0f };  // running avg of grad^2
    std::vector<float> edx2    = { 0.0f, 0.0f, 0.0f, 0.0f };  // running avg of delta^2

    // Optimization loop
    for (int it = 0; it < steps; ++it) {
        // ---- compute gradients (quadratic loss L = sum w_i^2) ----
        float g0 = 2.0f * param[0];
        float g1 = 2.0f * param[1];
        float g2 = 2.0f * param[2];
        float g3 = 2.0f * param[3];

        // ---- update running average of squared gradients ----
        float tmp0 = g0 * g0;
        float tmp1 = g1 * g1;
        float tmp2 = g2 * g2;
        float tmp3 = g3 * g3;

        eg2[0] = rho * eg2[0] + (1.0f - rho) * tmp0;
        eg2[1] = rho * eg2[1] + (1.0f - rho) * tmp1;
        eg2[2] = rho * eg2[2] + (1.0f - rho) * tmp2;
        eg2[3] = rho * eg2[3] + (1.0f - rho) * tmp3;

        // ---- compute RMS values ----
        float rmsdx0 = std::sqrt(edx2[0] + eps);
        float rmsdx1 = std::sqrt(edx2[1] + eps);
        float rmsdx2 = std::sqrt(edx2[2] + eps);
        float rmsdx3 = std::sqrt(edx2[3] + eps);

        float rmsg0 = std::sqrt(eg2[0] + eps);
        float rmsg1 = std::sqrt(eg2[1] + eps);
        float rmsg2 = std::sqrt(eg2[2] + eps);
        float rmsg3 = std::sqrt(eg2[3] + eps);

        // ---- compute parameter updates ----
        float upd0 = - (rmsdx0 / rmsg0) * g0;
        float upd1 = - (rmsdx1 / rmsg1) * g1;
        float upd2 = - (rmsdx2 / rmsg2) * g2;
        float upd3 = - (rmsdx3 / rmsg3) * g3;

        // ---- apply updates ----
        param[0] += upd0;
        param[1] += upd1;
        param[2] += upd2;
        param[3] += upd3;

        // ---- update running average of squared updates ----
        float upd_sq0 = upd0 * upd0;
        float upd_sq1 = upd1 * upd1;
        float upd_sq2 = upd2 * upd2;
        float upd_sq3 = upd3 * upd3;

        edx2[0] = rho * edx2[0] + (1.0f - rho) * upd_sq0;
        edx2[1] = rho * edx2[1] + (1.0f - rho) * upd_sq1;
        edx2[2] = rho * edx2[2] + (1.0f - rho) * upd_sq2;
        edx2[3] = rho * edx2[3] + (1.0f - rho) * upd_sq3;
    }

    // Print final parameters
    std::cout << "Final parameters after AdaDelta:" << std::endl;
    for (size_t i = 0; i < param.size(); ++i) {
        std::cout << "w[" << i << "] = " << param[i] << std::endl;
    }

    return 0;
}
