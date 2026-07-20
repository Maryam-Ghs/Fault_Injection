#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>

/* LLM input variant 5: duplicate-heavy */

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

        // initialise moment vectors on first call
        if (m_vec.empty()) {
            int sz = (int)weights.size();
            m_vec.assign(sz, 0.0f);
            u_vec.assign(sz, 0.0f);
        }

        // compute bias‑corrected learning rate (same for all parameters)
        float pow_beta = powf(beta_one, (float)step_cnt);
        float lr_t = alpha / (1.0f - pow_beta);

        // reverse index loop – a different loop structure
        for (int idx = (int)weights.size() - 1; idx >= 0; --idx) {
            // ------- temporary variables -------
            float cur_grad = grads[idx];
            float abs_grad = cur_grad >= 0.0f ? cur_grad : -cur_grad;

            float m_prev = m_vec[idx];
            float u_prev = u_vec[idx];

            // first moment update
            float m_new = beta_one * m_prev + (1.0f - beta_one) * cur_grad;

            // infinity norm update
            float scaled_u = beta_two * u_prev;
            float u_new = scaled_u > abs_grad ? scaled_u : abs_grad;

            // store updated moments
            m_vec[idx] = m_new;
            u_vec[idx] = u_new;

            // parameter update
            float denom = u_new + epsilon;
            float delta = lr_t * (m_new / denom);
            weights[idx] = weights[idx] - delta;
        }
    }

private:
    float alpha;       // base learning rate
    float beta_one;    // decay rate for first moment
    float beta_two;    // decay rate for infinity norm
    float epsilon;     // small constant
    int step_cnt;      // time step

    std::vector<float> m_vec; // first moment estimates
    std::vector<float> u_vec; // infinity norm estimates
};

int main() {
    std::srand(42);                     // deterministic “random” data
    int dim = 6;                        // number of parameters
    std::vector<float> w(dim);
    std::vector<float> g(dim);

    // duplicate‑heavy initial weights
    w[0] = 0.0f;
    w[1] = 0.0f;
    w[2] = 5.0f;
    w[3] = 5.0f;
    w[4] = -5.0f;
    w[5] = -5.0f;

    // duplicate‑heavy gradient vector
    g[0] = 0.0f;
    g[1] = 0.0f;
    g[2] = 1e3f;
    g[3] = 1e3f;
    g[4] = -1e3f;
    g[5] = -1e3f;

    // AdaMax hyper‑parameters (edge‑case friendly)
    AdaMaxOpt optimizer(0.002f, 0.9f, 0.999f, 1e-8f);

    // run several steps to see the effect of duplicate‑heavy data
    for (int it = 0; it < 12; ++it) {
        optimizer.step(w, g);
    }

    // print final parameters
    std::cout << "Final parameters after AdaMax updates:\n";
    for (int i = 0; i < dim; ++i) {
        std::cout << "w[" << i << "] = " << w[i] << "\n";
    }
    return 0;
}
