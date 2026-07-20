#include <iostream>
#include <cmath>

/* LLM input variant 5: duplicate-heavy */

int main() {
    // ---------- hyper‑parameters ----------
    float lr   = 0.002f;      // learning rate
    float b1   = 0.9f;        // beta1
    float b2   = 0.999f;      // beta2
    float eps  = 1e-8f;       // epsilon for numerical stability

    // ---------- problem set‑up ----------
    const int dim = 4;                     // number of parameters (stack array)
    float theta[dim] = { 1.0f, 1.0f, 1.0f, 1.0f };   // initial parameters (all identical)
    float m[dim] = { 0.0f, 0.0f, 0.0f, 0.0f };        // first moment estimate
    float u[dim] = { 0.0f, 0.0f, 0.0f, 0.0f };        // infinity norm estimate

    // ---------- artificially heavy gradient sequence ----------
    // 5 steps, each step provides a gradient vector with many duplicate values
    float grads[5][dim] = {
        { 1000.0f, 1000.0f, -1000.0f, -1000.0f },
        { 1000.0f, 1000.0f, -1000.0f, -1000.0f },
        { 1000.0f, 1000.0f, -1000.0f, -1000.0f },
        { 1000.0f, 1000.0f, -1000.0f, -1000.0f },
        { 1000.0f, 1000.0f, -1000.0f, -1000.0f }
    };

    // ---------- optimizer loop ----------
    float b1_pow = 1.0f;   // will hold b1^t
    for (int step = 1; step <= 5; ++step) {
        // update bias‑correction term (reordered multiplication)
        b1_pow = b1_pow * b1;                     // b1_pow = b1^step

        // ---------- manual unrolled update for each dimension ----------
        // ---- element 0 ----
        float g0 = grads[step-1][0];
        float abs_g0 = (g0 >= 0.0f) ? g0 : -g0;
        m[0] = b1 * m[0] + (1.0f - b1) * g0;       // first moment
        float ub0 = b2 * u[0];
        u[0] = (ub0 > abs_g0) ? ub0 : abs_g0;     // infinity norm
        float lr_corr0 = lr / (1.0f - b1_pow);    // bias‑corrected lr
        float delta0 = lr_corr0 * (m[0] / (u[0] + eps));
        theta[0] = theta[0] - delta0;

        // ---- element 1 ----
        float g1 = grads[step-1][1];
        float abs_g1 = (g1 >= 0.0f) ? g1 : -g1;
        m[1] = b1 * m[1] + (1.0f - b1) * g1;
        float ub1 = b2 * u[1];
        u[1] = (ub1 > abs_g1) ? ub1 : abs_g1;
        float lr_corr1 = lr / (1.0f - b1_pow);
        float delta1 = lr_corr1 * (m[1] / (u[1] + eps));
        theta[1] = theta[1] - delta1;

        // ---- element 2 ----
        float g2 = grads[step-1][2];
        float abs_g2 = (g2 >= 0.0f) ? g2 : -g2;
        m[2] = b1 * m[2] + (1.0f - b1) * g2;
        float ub2 = b2 * u[2];
        u[2] = (ub2 > abs_g2) ? ub2 : abs_g2;
        float lr_corr2 = lr / (1.0f - b1_pow);
        float delta2 = lr_corr2 * (m[2] / (u[2] + eps));
        theta[2] = theta[2] - delta2;

        // ---- element 3 ----
        float g3 = grads[step-1][3];
        float abs_g3 = (g3 >= 0.0f) ? g3 : -g3;
        m[3] = b1 * m[3] + (1.0f - b1) * g3;
        float ub3 = b2 * u[3];
        u[3] = (ub3 > abs_g3) ? ub3 : abs_g3;
        float lr_corr3 = lr / (1.0f - b1_pow);
        float delta3 = lr_corr3 * (m[3] / (u[3] + eps));
        theta[3] = theta[3] - delta3;
    }

    // ---------- output ----------
    std::cout << "Final parameters after AdaMax (version #5):\n";
    for (int i = 0; i < dim; ++i) {
        std::cout << "theta[" << i << "] = " << theta[i] << "\n";
    }
    return 0;
}
