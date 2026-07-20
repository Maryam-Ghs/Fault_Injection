#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>

/* LLM input variant 10: large-safe-stress */

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
// deterministic test harness – large-safe-stress variant
int main() {
    const int DIM = 200;      // number of parameters (large but safe)
    const int STEPS = 200;    // number of gradient steps

    // initial parameters (size DIM)
    std::vector<float> theta;
    theta.resize(DIM);
    int j = 0;
    while (j < DIM) {
        // repeat the original three‑value pattern across the larger vector
        if (j % 3 == 0) theta[j] = 0.5f;
        else if (j % 3 == 1) theta[j] = -0.3f;
        else theta[j] = 0.8f;
        j = j + 1;
    }

    // generate deterministic gradient sequence (STEPS steps)
    std::vector< std::vector<float> > grads;
    int step = 0;
    while (step < STEPS) {
        std::vector<float> g;
        g.resize(DIM);
        int i = 0;
        while (i < DIM) {
            // gradient magnitude grows slowly with step, alternating sign by index
            float base = (step + 1) * 0.001f;
            g[i] = (i % 2 == 0) ? base : -base;
            i = i + 1;
        }
        grads.push_back(g);
        step = step + 1;
    }

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
