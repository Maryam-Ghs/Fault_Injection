#include <iostream>
#include <cmath>

/* LLM input variant 8: sparse-skewed */

class AdamW {
public:
    AdamW(int sz, float lr_, float wd_, float b1_, float b2_, float eps_)
        : dim(sz), lr(lr_), wd(wd_), beta1(b1_), beta2(b2_), eps(eps_),
          m(new float[sz]), v(new float[sz]),
          beta1_pow(1.0f), beta2_pow(1.0f)
    {
        for (int i = 0; i < dim; ++i) {
            m[i] = 0.0f;
            v[i] = 0.0f;
        }
    }

    ~AdamW() {
        delete[] m;
        delete[] v;
    }

    void step(float* w, const float* g) {
        beta1_pow *= beta1;               // update bias‑correction powers
        beta2_pow *= beta2;

        for (int i = 0; i < dim; ++i) {
            // moment updates
            float g_i = g[i];
            float m_i = beta1 * m[i] + (1.0f - beta1) * g_i;
            float v_i = beta2 * v[i] + (1.0f - beta2) * g_i * g_i;

            // bias‑corrected moments
            float m_hat = m_i / (1.0f - beta1_pow);
            float v_hat = v_i / (1.0f - beta2_pow);

            // denominator (sqrt + eps) – split into temp
            float sqrt_v = std::sqrt(v_hat);
            float denom = sqrt_v + eps;

            // AdamW update (weight decay applied after gradient step)
            float step_val = lr * (m_hat / denom);
            float decay_val = lr * wd * w[i];
            w[i] = w[i] - step_val - decay_val;

            // store back updated moments
            m[i] = m_i;
            v[i] = v_i;
        }
    }

private:
    int dim;
    float lr, wd, beta1, beta2, eps;
    float* m;
    float* v;
    float beta1_pow, beta2_pow;
};

int main() {
    const int N = 20;                    // larger dimension, mostly sparse
    float* w = new float[N];
    float* g = new float[N];

    // Sparse initialization: only a few parameters are non‑zero
    for (int i = 0; i < N; ++i) w[i] = 0.0f;
    w[0] = 0.5f;   // cluster 1
    w[5] = -0.3f;  // cluster 2
    w[10] = 1.0f;  // isolated

    // Gradient vector initially zero everywhere
    for (int i = 0; i < N; ++i) g[i] = 0.0f;
    // Edge‑case heavy hyper‑parameters
    float learning_rate = 0.1f;
    float weight_decay   = 10.0f;   // unusually large decay
    float beta_one       = 0.9f;
    float beta_two       = 0.999f;
    float epsilon        = 0.0f;   // epsilon = 0 to expose division edge

    AdamW optimizer(N, learning_rate, weight_decay, beta_one, beta_two, epsilon);

    const int STEPS = 5;
    for (int s = 0; s < STEPS; ++s) {
        // Sparse, skewed gradient updates: only few indices receive non‑zero values
        for (int i = 0; i < N; ++i) g[i] = 0.0f; // reset to zero

        // Toggle a tiny gradient at index 0
        g[0] = (s % 2 == 0) ? 0.0f : 0.05f;

        // Decreasing gradient at index 5
        g[5] = 0.1f * (1.0f - 0.2f * s);

        // Increasing magnitude at index 10
        g[10] = -0.2f * (1.0f + 0.1f * s);

        // Occasionally inject a small gradient far in the vector (sparse outlier)
        if (s == 2) g[15] = 0.03f;

        optimizer.step(w, g);
    }

    // Output final parameters
    std::cout << "Final parameters:" << std::endl;
    for (int i = 0; i < N; ++i) {
        std::cout << "w[" << i << "] = " << w[i] << std::endl;
    }

    delete[] w;
    delete[] g;
    return 0;
}
