#include <iostream>
#include <cmath>

/* LLM input variant 8: sparse-skewed */

int main() {
    /*--- hyper‑parameters (float only) ---*/
    float lr = 0.01f;          // learning rate
    float b1 = 0.9f;           // beta1
    float b2 = 0.999f;         // beta2
    float eps = 1e-8f;         // epsilon
    float wd = 0.01f;          // weight‑decay

    /*--- problem size (stack arrays) ---*/
    const int N = 12;          // number of parameters (sparse)
    const int T = 4;           // number of update steps

    /*--- initialise parameters with sparse, skewed values ---*/
    float w[N] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1e6f, 0.0f, 0.0f, 0.0f, -1e6f, 0.0f, 0.0f};
    float g[N] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 5e7f, 0.0f, 0.0f, 0.0f, -5e7f, 0.0f, 0.0f};

    /*--- first‑moment and second‑moment buffers ---*/
    float m[N] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    float v[N] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

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
