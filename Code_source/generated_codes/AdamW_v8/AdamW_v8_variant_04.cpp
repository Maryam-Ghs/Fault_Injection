#include <iostream>
#include <cmath>

/* LLM input variant 4: signed-extremes */
int main() {
    /*--- hyper‑parameters (float only) ---*/
    float lr = 0.02f;          // learning rate (increased)
    float b1 = 0.85f;          // beta1 (slightly lower)
    float b2 = 0.990f;         // beta2 (slightly lower)
    float eps = 1e-6f;         // epsilon (larger to stay clear of zero)
    float wd = -0.02f;         // weight‑decay (negative to test signed extreme)

    /*--- problem size (stack arrays) ---*/
    const int N = 5;           // number of parameters
    const int T = 4;           // number of update steps (extended)

    /*--- initialise parameters with signed‑extreme values ---*/
    float w[N] = {
        -1e30f,   // large negative
        0.0f,     // zero
        1e30f,    // large positive
        -1e-30f,  // tiny negative
        1e-30f    // tiny positive
    };
    float g[N] = {
        0.0f,          // zero gradient
        -1e19f,        // large negative gradient
        1e19f,         // large positive gradient
        -1e-19f,       // tiny negative gradient
        1e-19f         // tiny positive gradient
    };

    /*--- first‑moment and second‑moment buffers ---*/
    float m[N] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    float v[N] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

    /*--- time‑step counter (float for powf) ---*/
    float step = 0.0f;

    /*--- optimisation loop (while, branch‑minimised) ---*/
    while (step < (float)T) {
        step = step + 1.0f;                     // t = t + 1

        /*--- compute bias‑correction factors (expanded) ---*/
        float pow_b1 = std::pow(b1, step);
        float pow_b2 = std::pow(b2, step);
        float corr1 = 1.0f - pow_b1;
        float corr2 = 1.0f - pow_b2;

        int i = 0;
        while (i < N) {
            /*--- update first‑moment estimate ---*/
            float grad_i = g[i];
            float m_prev = m[i];
            float m_tmp = b1 * m_prev;
            float m_inc = (1.0f - b1) * grad_i;
            m[i] = m_tmp + m_inc;               // m_t

            /*--- update second‑moment estimate ---*/
            float v_prev = v[i];
            float sqr_grad = grad_i * grad_i;
            float v_tmp = b2 * v_prev;
            float v_inc = (1.0f - b2) * sqr_grad;
            v[i] = v_tmp + v_inc;               // v_t

            /*--- bias‑corrected moments (multi‑step) ---*/
            float m_hat = m[i] / corr1;
            float v_hat = v[i] / corr2;

            /*--- denominator (sqrt + eps) ---*/
            float sqrt_v = std::sqrt(v_hat);
            float denom = sqrt_v + eps;

            /*--- weight‑decay term (expanded) ---*/
            float wd_term = wd * w[i];

            /*--- final update (reordered) ---*/
            float step_lr = lr * (m_hat / denom);
            float delta = step_lr + wd_term;
            w[i] = w[i] - delta;                // param ← param – delta

            i = i + 1;                           // next index
        }
    }

    /*--- output final parameters (edge‑case aware) ---*/
    std::cout << "Final parameters after " << (int)step << " steps:\n";
    for (int j = 0; j < N; ++j) {
        std::cout << "w[" << j << "] = " << w[j] << '\n';
    }
    return 0;
}
