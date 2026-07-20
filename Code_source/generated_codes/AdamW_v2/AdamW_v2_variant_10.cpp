#include <iostream>
#include <cmath>

/* LLM input variant 10: large-safe-stress */

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

            // denominator (sqrt + eps)
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
    const int N = 10000;                 // large but safe number of parameters
    float* w = new float[N];
    float* g = new float[N];

    // Deterministic heavy initialization
    for (int i = 0; i < N; ++i) {
        w[i] = (i % 2 == 0) ? 0.5f : -0.5f;
        g[i] = 0.0f;
    }

    // AdamW hyper‑parameters (large‑scale safe values)
    float learning_rate = 0.01f;
    float weight_decay   = 0.1f;
    float beta_one       = 0.9f;
    float beta_two       = 0.999f;
    float epsilon        = 1e-8f;   // small epsilon to avoid division by zero

    AdamW optimizer(N, learning_rate, weight_decay, beta_one, beta_two, epsilon);

    const int STEPS = 100;
    for (int s = 0; s < STEPS; ++s) {
        // deterministic gradient pattern
        for (int i = 0; i < N; ++i) {
            // values cycle between -0.04 and 0.04
            g[i] = 0.01f * ((i + s) % 10 - 5);
        }
        optimizer.step(w, g);
    }

    // Output final parameters (first 10 for brevity)
    std::cout << "Final parameters (first 10):" << std::endl;
    for (int i = 0; i < 10; ++i) {
        std::cout << "w[" << i << "] = " << w[i] << std::endl;
    }

    delete[] w;
    delete[] g;
    return 0;
}
