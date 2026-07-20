// LLM input variant 10: large-safe-stress
// Adam Optimizer – version #6
// ------------------------------------------------------------
// Implements a minimal deterministic test of Adam using only
// float and int, stack arrays, helper functions and a branch‑
// minimized style.
// ------------------------------------------------------------

#include <iostream>
#include <cmath>

// -----------------------------------------------------------------
// Fill a stack array with a constant value (no branching)
// -----------------------------------------------------------------
void fill_arr(int n, float *a, float v) {
    int i = 0;
    while (i < n) {
        a[i] = v;
        ++i;
    }
}

// -----------------------------------------------------------------
// One Adam update step (all temporaries are explicit)
// -----------------------------------------------------------------
void adam_step(
    int n,                // dimensionality
    float *w,             // parameters (in‑out)
    const float *dw,      // gradients (in)
    float *mt,            // first moment (in‑out)
    float *vt,            // second moment (in‑out)
    int step,             // time step (1‑based)
    float lr,             // learning rate
    float b1,             // beta1
    float b2,             // beta2
    float eps)            // epsilon
{
    // bias‑correction denominators (pre‑computed powers)
    float pow_b1 = std::pow(b1, (float)step);
    float pow_b2 = std::pow(b2, (float)step);
    float den1 = 1.0f - pow_b1;
    float den2 = 1.0f - pow_b2;

    // loop without explicit branches
    int i = 0;
    while (i < n) {
        // ---- first‑moment update ----
        float grad_i = dw[i];
        float m_prev = mt[i];
        float m_tmp  = b1 * m_prev + (1.0f - b1) * grad_i;

        // ---- second‑moment update ----
        float v_prev = vt[i];
        float sq_grad = grad_i * grad_i;
        float v_tmp   = b2 * v_prev + (1.0f - b2) * sq_grad;

        // ---- bias‑corrected estimates ----
        float m_hat = m_tmp / den1;
        float v_hat = v_tmp / den2;

        // ---- parameter update ----
        float step_size = lr / (std::sqrt(v_hat) + eps);
        float w_new = w[i] - step_size * m_hat;

        // store back
        mt[i] = m_tmp;
        vt[i] = v_tmp;
        w[i]  = w_new;

        ++i;
    }
}

// -----------------------------------------------------------------
// Main – deterministic test vectors, no external input
// -----------------------------------------------------------------
int main() {
    // problem size (large but safe for stack)
    const int DIM = 500;

    // parameters (weights) – deterministic start
    float w[DIM];
    // initialize w with a repeating pattern
    for (int i = 0; i < DIM; ++i) {
        if (i % 3 == 0) w[i] = 0.5f;
        else if (i % 3 == 1) w[i] = -0.3f;
        else w[i] = 0.8f;
    }

    // first‑ and second‑moment buffers
    float m_buf[DIM];
    float v_buf[DIM];
    fill_arr(DIM, m_buf, 0.0f);
    fill_arr(DIM, v_buf, 0.0f);

    // hyper‑parameters
    float lr   = 0.01f;
    float b1   = 0.9f;
    float b2   = 0.999f;
    float eps  = 1e-8f;

    // deterministic gradient schedule (linear growth)
    const int total_steps = 1000;
    int t = 1;
    while (t <= total_steps) {
        float grad[DIM];
        for (int i = 0; i < DIM; ++i) {
            // simple linear growth scaled by index to keep values moderate
            grad[i] = (0.001f * (float)(i + 1)) * (float)t;
        }

        adam_step(DIM, w, grad, m_buf, v_buf, t, lr, b1, b2, eps);
        ++t;
    }

    // output final parameters (show first 10 for brevity)
    std::cout << "Final parameters after " << total_steps << " Adam steps (showing first 10):\n";
    for (int i = 0; i < 10; ++i) {
        std::cout << "w[" << i << "] = " << w[i] << '\n';
    }
    return 0;
}
