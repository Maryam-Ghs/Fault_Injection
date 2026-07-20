#include <iostream>
#include <cmath>

/* LLM input variant 7: reverse-adversarial */

class AdamOpt {
public:
    float *mom;          // first moment vector
    float *var;          // second moment vector
    float step_lr;       // learning rate
    float beta_one;      // exponential decay rate for the first moment
    float beta_two;      // exponential decay rate for the second moment
    float eps;           // small constant to avoid division by zero
    int   iter;          // time step
    int   dim;           // number of parameters

    AdamOpt(int d, float lr, float b1, float b2, float e) {
        dim = d;
        step_lr = lr;
        beta_one = b1;
        beta_two = b2;
        eps = e;
        iter = 0;
        mom = new float[dim];
        var = new float[dim];
        int idx = 0;
        while (idx < dim) {
            mom[idx] = 0.0f;
            var[idx] = 0.0f;
            ++idx;
        }
    }

    ~AdamOpt() {
        delete[] mom;
        delete[] var;
    }

    void update(float *param, float *grad) {
        ++iter;                                   // increase time step
        int i = 0;
        while (i < dim) {
            // ----- first moment update -----
            float old_m = mom[i] * beta_one;                 // beta1 * m_{t-1}
            float grad_contrib = (1.0f - beta_one) * grad[i]; // (1-beta1) * g_t
            mom[i] = old_m + grad_contrib;                    // m_t

            // ----- second moment update -----
            float old_v = var[i] * beta_two;                  // beta2 * v_{t-1}
            float grad_sq = grad[i] * grad[i];                // g_t^2
            float var_contrib = (1.0f - beta_two) * grad_sq;  // (1-beta2) * g_t^2
            var[i] = old_v + var_contrib;                     // v_t

            // ----- bias‑corrected moments -----
            float pow_b1 = powf(beta_one, (float)iter);
            float pow_b2 = powf(beta_two, (float)iter);
            float m_hat = mom[i] / (1.0f - pow_b1);
            float v_hat = var[i] / (1.0f - pow_b2);

            // ----- parameter update -----
            float denom = sqrtf(v_hat) + eps;                 // sqrt(v̂) + ε
            float step = step_lr * m_hat / denom;             // learning step
            param[i] = param[i] - step;                       // θ_t = θ_{t-1} - step

            ++i;
        }
    }
};

int main() {
    // ----- generate a deterministic, reverse‑ordered worst‑case problem -----
    const int size = 80;                               // moderate number of parameters
    float *theta = new float[size];                   // parameters
    float *delta = new float[size];                   // gradients

    // Initialize parameters in strictly decreasing order from 1.0 down to ~0.25
    for (int i = 0; i < size; ++i) {
        theta[i] = 1.0f - static_cast<float>(i) / static_cast<float>(size) * 0.75f;
    }

    // Initialize gradients to alternating large magnitudes, also in reverse order
    for (int i = 0; i < size; ++i) {
        float magnitude = 1.0f - static_cast<float>(i) / static_cast<float>(size) * 0.9f; // from ~1.0 down to ~0.1
        delta[i] = (i % 2 == 0) ? magnitude : -magnitude;
    }

    // ----- create Adam optimizer instance -----
    AdamOpt solver(size, 0.01f, 0.9f, 0.999f, 1e-8f);

    // ----- run a few optimization steps with deterministic gradient changes -----
    const int rounds = 12;
    for (int r = 0; r < rounds; ++r) {
        // Simulate a systematic gradient shift: flip sign and slightly reduce magnitude each round
        for (int k = 0; k < size; ++k) {
            float base = 1.0f - static_cast<float>(k) / static_cast<float>(size) * 0.9f;
            float decay = 1.0f - static_cast<float>(r) * 0.02f; // slowly decay over rounds
            delta[k] = ((r + k) % 2 == 0 ? 1 : -1) * base * decay;
        }
        solver.update(theta, delta);
    }

    // ----- output first few optimized parameters -----
    for (int out = 0; out < 5; ++out) {
        std::cout << theta[out] << ' ';
    }
    std::cout << std::endl;

    delete[] theta;
    delete[] delta;
    return 0;
}
