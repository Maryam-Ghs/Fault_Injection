/* LLM input variant 7: reverse-adversarial */
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
    // problem size (larger and reverse‑ordered)
    const int dim = 10;

    // parameters (weights) – descending values to create reverse ordering
    float w[dim] = { 1.0f, 0.9f, 0.8f, 0.7f, 0.6f, 0.5f, 0.4f, 0.3f, 0.2f, 0.1f };

    // first‑ and second‑moment buffers
    float m_buf[dim];
    float v_buf[dim];
    fill_arr(dim, m_buf, 0.0f);
    fill_arr(dim, v_buf, 0.0f);

    // hyper‑parameters (edge‑case values)
    const float lr   = 0.01f;
    const float b1   = 0.999f;
    const float b2   = 0.9999f;
    const float eps  = 1e-8f;

    // deterministic gradient schedule (reverse linear decay)
    const int total_steps = 8;
    int t = 1;
    while (t <= total_steps) {
        float grad[dim];
        // larger magnitudes early, decreasing each step
        float scale = (float)(total_steps - t + 1) * 0.05f;
        int i = 0;
        while (i < dim) {
            // reverse‑ordered gradient: highest index gets smallest magnitude
            grad[i] = ((float)(dim - i)) * scale * ((i % 2 == 0) ? 1.0f : -1.0f);
            ++i;
        }

        adam_step(dim, w, grad, m_buf, v_buf, t, lr, b1, b2, eps);
        ++t;
    }

    // output final parameters
    std::cout << "Final parameters after " << total_steps << " Adam steps:\n";
    int i = 0;
    while (i < dim) {
        std::cout << "w[" << i << "] = " << w[i] << '\n';
        ++i;
    }
    return 0;
}
