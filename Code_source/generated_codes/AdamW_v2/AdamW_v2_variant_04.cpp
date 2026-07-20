#include <iostream>
#include <cmath>

/* LLM input variant 4: signed-extremes */

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
        beta1_pow *= beta1;               
        beta2_pow *= beta2;

        for (int i = 0; i < dim; ++i) {
            float g_i = g[i];
            float m_i = beta1 * m[i] + (1.0f - beta1) * g_i;
            float v_i = beta2 * v[i] + (1.0f - beta2) * g_i * g_i;

            float m_hat = m_i / (1.0f - beta1_pow);
            float v_hat = v_i / (1.0f - beta2_pow);

            float sqrt_v = std::sqrt(v_hat);
            float denom = sqrt_v + eps;

            float step_val = lr * (m_hat / denom);
            float decay_val = lr * wd * w[i];
            w[i] = w[i] - step_val - decay_val;

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
    const int N = 5;                     // expanded number of parameters
    float* w = new float[N];
    float* g = new float[N];

    // Mixed-sign initialization
    w[0] = -1.5f;   // negative
    w[1] =  0.0f;   // zero
    w[2] =  2.0f;   // positive large
    w[3] = -0.7f;   // negative small
    w[4] =  0.3f;   // positive small

    // Initial gradients (mixed signs, include zero)
    g[0] =  0.0f;
    g[1] =  0.05f;
    g[2] = -0.1f;
    g[3] =  0.0f;
    g[4] =  0.07f;

    // AdamW hyper‑parameters (safe epsilon)
    float learning_rate = 0.05f;
    float weight_decay   = 1.0f;
    float beta_one       = 0.85f;
    float beta_two       = 0.995f;
    float epsilon        = 1e-8f;

    AdamW optimizer(N, learning_rate, weight_decay, beta_one, beta_two, epsilon);

    const int STEPS = 6;
    for (int s = 0; s < STEPS; ++s) {
        // Vary gradients: mix negative, zero, positive without branching
        g[0] = (s % 3 == 0) ? -0.02f : ((s % 3 == 1) ? 0.0f : 0.03f);
        g[1] = 0.05f * (1.0f - 0.1f * s);          // decreasing positive
        g[2] = -0.1f * (1.0f + 0.2f * s);          // increasing magnitude negative
        g[3] = (s % 2 == 0) ? 0.0f : -0.04f;       // toggle zero / negative
        g[4] = 0.07f * ((s % 4) - 1);              // -0.07, 0, 0.07, 0.14 pattern

        optimizer.step(w, g);
    }

    std::cout << "Final parameters:" << std::endl;
    for (int i = 0; i < N; ++i) {
        std::cout << "w[" << i << "] = " << w[i] << std::endl;
    }

    delete[] w;
    delete[] g;
    return 0;
}
