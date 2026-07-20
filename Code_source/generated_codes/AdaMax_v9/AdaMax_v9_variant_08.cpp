#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>

/* LLM input variant 8: sparse-skewed */

class AdaMaxOptimizer {
public:
    // hyper‑parameters (all float, no const)
    float lr_val;
    float beta1_val;
    float beta2_val;
    float eps_val;

    // internal state
    std::vector<float> m_vec;   // first moment
    std::vector<float> u_vec;   // infinity norm
    int step_cnt;               // time step

    // constructor – size of parameter vector
    AdaMaxOptimizer(int dim) {
        lr_val = 0.01f;
        beta1_val = 0.9f;
        beta2_val = 0.999f;
        eps_val = 1e-8f;

        m_vec.resize(dim);
        u_vec.resize(dim);
        // initialise vectors to zero without using const or double
        int i = 0;
        while (i < dim) {
            m_vec[i] = 0.0f;
            u_vec[i] = 0.0f;
            i = i + 1;
        }
        step_cnt = 0;
    }

    // one optimisation step
    void step(std::vector<float>& param, const std::vector<float>& grad) {
        // increase time step
        step_cnt = step_cnt + 1;

        // update moments – use while loops for variety
        int idx = 0;
        while (idx < (int)param.size()) {
            // m_t = beta1 * m_{t-1} + (1-beta1) * g_t
            m_vec[idx] = beta1_val * m_vec[idx] + (1.0f - beta1_val) * grad[idx];

            // u_t = max(beta2 * u_{t-1}, |g_t|)
            float abs_g = std::abs(grad[idx]);
            float scaled_u = beta2_val * u_vec[idx];
            u_vec[idx] = (scaled_u > abs_g) ? scaled_u : abs_g;

            // bias‑corrected first moment
            float bias_correction = 1.0f - std::pow(beta1_val, (float)step_cnt);
            float m_hat = m_vec[idx] / bias_correction;

            // parameter update – order of operations reordered
            float denom = u_vec[idx] + eps_val;
            float delta = (lr_val / denom) * m_hat;
            param[idx] = param[idx] - delta;

            idx = idx + 1;
        }
    }
};

// ------------------------------------------------------------
// deterministic test harness – version #9
int main() {
    // initial parameters (size 20, mostly zeros)
    std::vector<float> theta;
    theta.resize(20);
    int j = 0;
    while (j < 20) {
        if (j == 0) theta[j] = 0.5f;          // a moderate positive value
        else if (j == 5) theta[j] = -0.3f;    // a moderate negative value
        else if (j == 15) theta[j] = 0.8f;    // another positive value
        else theta[j] = 0.0f;                  // sparse zeros elsewhere
        j = j + 1;
    }

    // fixed gradient sequence (5 steps, sparse and skewed)
    std::vector< std::vector<float> > grads;
    grads.push_back({
        0.1f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.2f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.05f, 0.0f, 0.0f, 0.0f, 0.0f
    });
    grads.push_back({
        0.05f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.1f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.02f, 0.0f, 0.0f, 0.0f, 0.0f
    });
    grads.push_back({
        0.02f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.04f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.01f, 0.0f, 0.0f, 0.0f, 0.0f
    });
    grads.push_back({
        0.01f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.02f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.005f, 0.0f, 0.0f, 0.0f, 0.0f
    });
    grads.push_back({
        0.005f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.01f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.002f, 0.0f, 0.0f, 0.0f, 0.0f
    });

    // create optimizer instance
    AdaMaxOptimizer opt((int)theta.size());

    // run optimisation, printing after each step
    int t = 0;
    while (t < (int)grads.size()) {
        opt.step(theta, grads[t]);

        std::cout << "Step " << (t + 1) << " parameters: ";
        int k = 0;
        while (k < (int)theta.size()) {
            std::cout << theta[k];
            if (k < (int)theta.size() - 1) std::cout << ", ";
            k = k + 1;
        }
        std::cout << std::endl;

        t = t + 1;
    }

    return 0;
}
