#include <iostream>
#include <cmath>

/* LLM input variant 6: ordered-structured */

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
    const int N = 5;                     // number of parameters (ordered)
    float* w = new float[N];             // parameters on heap
    float* g = new float[N];             // gradients on heap

    // Structured, sorted initialization
    w[0] = -2.0f; w[1] = -1.0f; w[2] = 0.0f; w[3] = 1.0f; w[4] = 2.0f;
    // Initial gradients follow a symmetric pattern
    g[0] = -0.2f; g[1] = -0.1f; g[2] = 0.0f; g[3] = 0.1f; g[4] = 0.2f;

    // AdamW hyper‑parameters (typical values)
    float learning_rate = 0.01f;
    float weight_decay   = 0.01f;
    float beta_one       = 0.9f;
    float beta_two       = 0.999f;
    float epsilon        = 1e-8f;

    AdamW optimizer(N, learning_rate, weight_decay, beta_one, beta_two, epsilon);

    const int STEPS = 5;
    for (int s = 0; s < STEPS; ++s) {
        // Structured gradient evolution: each component scales linearly with step
        for (int i = 0; i < N; ++i) {
            float base = static_cast<float>(i - 2); // -2,-1,0,1,2
            g[i] = base * 0.05f * (1.0f + 0.1f * s);
        }
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
