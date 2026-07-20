#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>

/* LLM input variant 5: duplicate-heavy */

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
    // ----- generate a medium‑sized deterministic duplicate‑heavy problem -----
    int size = 80;                               // moderate number of parameters
    float *theta = new float[size];              // parameters
    float *delta = new float[size];              // gradients

    int j = 0;
    while (j < size) {
        theta[j] = 0.5f;          // all parameters start with the same value
        delta[j] = 0.1f;          // all gradients start with the same value
        ++j;
    }

    // ----- create Adam optimizer instance -----
    AdamOpt solver(size, 0.01f, 0.9f, 0.999f, 1e-8f);

    // ----- run a few optimization steps with unchanged duplicate gradients -----
    int rounds = 12;
    int r = 0;
    while (r < rounds) {
        // keep the gradient identical across all parameters and iterations
        int k = 0;
        while (k < size) {
            delta[k] = 0.1f;
            ++k;
        }
        solver.update(theta, delta);
        ++r;
    }

    // ----- output first few optimized parameters -----
    int out = 0;
    while (out < 5) {
        std::cout << theta[out] << ' ';
        ++out;
    }
    std::cout << std::endl;

    delete[] theta;
    delete[] delta;
    return 0;
}
