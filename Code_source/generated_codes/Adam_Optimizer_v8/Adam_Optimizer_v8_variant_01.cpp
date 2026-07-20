/* LLM input variant 1: minimal-boundary */
// Adam Optimizer implementation – version #8
// Everything is placed inside main(), using only stack arrays and float/int types.

#include <cstdio>
#include <cstdlib>
#include <cmath>

#define SZ 1        // minimal nontrivial size of the parameter vector

int main() {
    // ------------------------------------------------------------
    // 1. Random seed and hyper‑parameters (float only)
    // ------------------------------------------------------------
    srand(0);                       // deterministic pseudo‑random numbers

    float learning_rate = 0.01f;    // step size
    float beta_one      = 0.9f;     // exponential decay rate for the 1st moment
    float beta_two      = 0.999f;   // exponential decay rate for the 2nd moment
    float epsilon       = 1e-8f;    // small constant to avoid division by zero

    // ------------------------------------------------------------
    // 2. Allocate all required vectors on the stack
    // ------------------------------------------------------------
    float param_vec[SZ];   // parameters to be optimised
    float grad_vec[SZ];    // gradients (synthetic for this demo)
    float moment1[SZ];     // first moment (m)
    float moment2[SZ];     // second moment (v)

    // ------------------------------------------------------------
    // 3. Initialise vectors with random values
    // ------------------------------------------------------------
    int idx = 0;
    while (idx < SZ) {
        // parameters in [0,1)
        param_vec[idx] = (float)rand() / (float)RAND_MAX;

        // gradients in [-1,1)
        grad_vec[idx]  = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;

        // moments start at zero
        moment1[idx] = 0.0f;
        moment2[idx] = 0.0f;

        ++idx;
    }

    // ------------------------------------------------------------
    // 4. Adam optimisation loop (few epochs for demonstration)
    // ------------------------------------------------------------
    int epoch = 0;
    int total_epochs = 1;                // minimal number of epochs

    while (epoch < total_epochs) {
        ++epoch;                         // time step t = epoch number

        // ---- bias‑correction factors (computed once per epoch) ----
        float pow_b1_t = 1.0f;            // beta_one^t
        float pow_b2_t = 1.0f;            // beta_two^t
        int cnt = 0;
        while (cnt < epoch) {
            pow_b1_t *= beta_one;
            pow_b2_t *= beta_two;
            ++cnt;
        }

        // ---- per‑parameter updates --------------------------------
        int i = 0;
        while (i < SZ) {
            // fetch the current gradient
            float cur_grad = grad_vec[i];

            // update first moment estimate (m)
            float one_minus_b1 = 1.0f - beta_one;
            float m_tmp = moment1[i] * beta_one;
            float m_inc = cur_grad * one_minus_b1;
            float m_new = m_tmp + m_inc;

            // update second moment estimate (v)
            float one_minus_b2 = 1.0f - beta_two;
            float v_tmp = moment2[i] * beta_two;
            float v_inc = cur_grad * cur_grad * one_minus_b2;
            float v_new = v_tmp + v_inc;

            // bias‑corrected moments (fused expression)
            float m_hat = m_new / (1.0f - pow_b1_t);
            float v_hat = v_new / (1.0f - pow_b2_t);

            // compute denominator (sqrt + epsilon) – fused
            float denom = sqrtf(v_hat) + epsilon;

            // parameter update (single fused line)
            param_vec[i] = param_vec[i] - learning_rate * m_hat / denom;

            // store back the updated moments
            moment1[i] = m_new;
            moment2[i] = v_new;

            ++i;
        }
    }

    // ------------------------------------------------------------
    // 5. Output a small slice of the final parameters
    // ------------------------------------------------------------
    printf("First 10 optimized parameters (epoch %d):\n", total_epochs);
    int j = 0;
    while (j < 10 && j < SZ) {
        printf("param[%d] = %.6f\n", j, param_vec[j]);
        ++j;
    }

    return 0;
}
