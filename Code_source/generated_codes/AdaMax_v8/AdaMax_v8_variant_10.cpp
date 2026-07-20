#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>

/* LLM input variant 10: large-safe-stress */

class AdaMaxOpt {
public:
    AdaMaxOpt(float lr_rate, float decay1, float decay2, float eps) {
        alpha = lr_rate;
        beta_one = decay1;
        beta_two = decay2;
        epsilon = eps;
        step_cnt = 0;
    }

    void step(std::vector<float>& weights, const std::vector<float>& grads) {
        ++step_cnt;

        if (m_vec.empty()) {
            int sz = static_cast<int>(weights.size());
            m_vec.assign(sz, 0.0f);
            u_vec.assign(sz, 0.0f);
        }

        float pow_beta = powf(beta_one, static_cast<float>(step_cnt));
        float lr_t = alpha / (1.0f - pow_beta);

        for (int idx = static_cast<int>(weights.size()) - 1; idx >= 0; --idx) {
            float cur_grad = grads[idx];
            float abs_grad = cur_grad >= 0.0f ? cur_grad : -cur_grad;

            float m_prev = m_vec[idx];
            float u_prev = u_vec[idx];

            float m_new = beta_one * m_prev + (1.0f - beta_one) * cur_grad;

            float scaled_u = beta_two * u_prev;
            float u_new = scaled_u > abs_grad ? scaled_u : abs_grad;

            m_vec[idx] = m_new;
            u_vec[idx] = u_new;

            float denom = u_new + epsilon;
            float delta = lr_t * (m_new / denom);
            weights[idx] = weights[idx] - delta;
        }
    }

private:
    float alpha;
    float beta_one;
    float beta_two;
    float epsilon;
    int step_cnt;

    std::vector<float> m_vec;
    std::vector<float> u_vec;
};

int main() {
    std::srand(42);                     // deterministic “random” data
    const int dim = 10000;               // large but safe number of parameters
    std::vector<float> w(dim);
    std::vector<float> g(dim);

    // initialise weights with a mix of normal, small, large, and extreme values
    for (int i = 0; i < dim; ++i) {
        // base random weight in [-10, 10]
        float base = static_cast<float>((std::rand() % 2000 - 1000)) / 100.0f;
        w[i] = base;
    }
    // inject a few edge‑case weights at known positions
    w[0] = 0.0f;                         // zero
    w[1] = 1e-6f;                        // very small
    w[2] = -1e6f;                        // large negative
    w[dim/2] = 5.0f;                     // normal
    w[dim/2 + 1] = -5.0f;                // normal negative
    w[dim-1] = 3.14159f;                 // pi

    // initialise gradients similarly, with extreme spikes
    for (int i = 0; i < dim; ++i) {
        float noise = static_cast<float>((std::rand() % 200 - 100)) / 10.0f; // moderate noise
        g[i] = noise;
    }
    // inject extreme gradients
    g[0] = 0.0f;                         // zero gradient
    g[1] = 1e8f;                         // huge positive gradient
    g[2] = -1e8f;                        // huge negative gradient
    g[dim/2] = 0.0f;                     // zero gradient in middle
    g[dim/2 + 1] = 1e8f;                // huge positive gradient in middle
    g[dim-1] = -1e8f;                    // huge negative gradient at end

    AdaMaxOpt optimizer(0.002f, 0.9f, 0.999f, 1e-8f);

    // run a modest number of steps to keep runtime reasonable
    for (int it = 0; it < 12; ++it) {
        optimizer.step(w, g);
    }

    // print a subset of final parameters to verify behavior
    std::cout << "Final parameters after AdaMax updates (sampled):\n";
    const int sample_interval = dim / 10; // show 10 samples
    for (int i = 0; i < dim; i += sample_interval) {
        std::cout << "w[" << i << "] = " << w[i] << "\n";
    }
    // also show the last element explicitly
    std::cout << "w[" << dim-1 << "] = " << w[dim-1] << "\n";
    return 0;
}
