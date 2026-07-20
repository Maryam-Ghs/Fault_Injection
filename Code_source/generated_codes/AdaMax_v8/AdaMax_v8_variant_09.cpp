#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>

/* LLM input variant 9: medium-deterministic-random */

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
    std::srand(12345);                     // deterministic pseudo‑random seed
    const int dim = 8;                      // medium‑sized parameter vector
    std::vector<float> w(dim);
    std::vector<float> g(dim);

    // deterministic diverse initial weights
    for (int i = 0; i < dim; ++i) {
        if (i == 2) {
            w[i] = 1e3f;                    // large positive outlier
        } else if (i == 5) {
            w[i] = -1e3f;                   // large negative outlier
        } else {
            w[i] = ((i % 2 == 0) ? 1.0f : -1.0f) * (i + 1) * 0.6f;
        }
    }

    // deterministic diverse gradient vector
    for (int i = 0; i < dim; ++i) {
        if (i == 1) {
            g[i] = 4.5e4f;                  // big positive gradient
        } else if (i == 6) {
            g[i] = -4.5e4f;                 // big negative gradient
        } else {
            g[i] = static_cast<float>((std::rand() % 200) - 100) / 20.0f; // range [-5,5]
        }
    }

    AdaMaxOpt optimizer(0.005f, 0.85f, 0.995f, 1e-7f);

    // execute a moderate number of steps
    for (int it = 0; it < 10; ++it) {
        optimizer.step(w, g);
    }

    std::cout << "Final parameters after AdaMax updates:\n";
    for (int i = 0; i < dim; ++i) {
        std::cout << "w[" << i << "] = " << w[i] << "\n";
    }
    return 0;
}
