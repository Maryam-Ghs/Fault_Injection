#include <iostream>
#include <cmath>

/* LLM input variant 7: reverse-adversarial */

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
    const int N = 5;                     // increased dimension for adversarial pattern
    float* w = new float[N];
    float* g = new float[N];

    // Reverse‑ordered heavy initialization (largest to smallest)
    w[0] = 5.0f; w[1] = 4.0f; w[2] = 3.0f; w[3] = 2.0f; w[4] = 1.0f;
    // Initial gradients set to descending magnitudes
    g[0] = 0.5f; g[1] = 0.4f; g[2] = 0.3f; g[3] = 0.2f; g[4] = 0.1f;

    // AdamW hyper‑parameters tuned for adversarial stress
    float learning_rate = 0.01f;
    float weight_decay   = 100.0f;   // extreme decay
    float beta_one       = 0.999f;   // very close to 1, slows moment decay
    float beta_two       = 0.9f;    // unusually low for second moment
    float epsilon        = 0.0f;    // no epsilon to expose division edge

    AdamW optimizer(N, learning_rate, weight_decay, beta_one, beta_two, epsilon);

    const int STEPS = 7;                // more steps for deeper adversarial progression
    for (int s = 0; s < STEPS; ++s) {
        // Reverse‑ordered gradient pattern: larger indices get smaller updates
        for (int i = 0; i < N; ++i) {
            // Gradients decrease both with index and over steps, creating a worst‑case monotonic decay
            float base = (N - i) * 0.1f;                 // reverse order magnitude
            float decay = 1.0f - 0.07f * s;              // step‑wise decay factor
            g[i] = base * decay;
        }
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
