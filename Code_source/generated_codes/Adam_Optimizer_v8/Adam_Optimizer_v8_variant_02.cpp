// Adam Optimizer implementation – version #8
// Everything is placed inside main(), using only stack arrays and float/int types.
/* LLM input variant 2: small-diverse */

#include <cstdio>
#include <cstdlib>
#include <cmath>

#define SZ 16        // reduced size for a small, diverse test case

int main() {
    // ------------------------------------------------------------
    // 1. Hyper‑parameters (float only)
    // ------------------------------------------------------------
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
    // 3. Initialise vectors with deterministic diverse values
    // ------------------------------------------------------------
    const float init_params[SZ] = {
        0.05f, 0.15f, 0.25f, 0.35f,
        0.45f, 0.55f, 0.65f, 0.75f,
        0.85f, 0.95f, 0.10f, 0.20f,
        0.30f, 0.40f, 0.50f, 0.60f
    };
    const float init_grads[SZ] = {
        -0.8f, 0.6f, -0.4f, 0.2f,
        -0.1f, 0.3f, -0.5f, 0.7f,
        -0.9f, 0.0f, 0.4f, -0.2f,
        0.8f, -0.6f, 0.5f, -0.3f
    };
    for (int i = 0; i < SZ; ++i) {
        param_vec[i] = init_params[i];
        grad_vec[i]  = init_grads[i];
        moment1[i] = 0.0f;
        moment2[i] = 0.0f;
    }

    // ------------------------------------------------------------
    // 4. Adam optimisation loop (few epochs for demonstration)
    // ------------------------------------------------------------
    int epoch = 0;
    int total_epochs = 5;                // small number to keep runtime tiny

    while (epoch < total_epochs) {
        ++epoch;                         // time step t = epoch number

        // ---- bias‑correction factors (computed once per epoch) ----
        float pow_b1_t = 1.0f;            // beta_one^t
        float pow_b2_t = 1.0f;            // beta_two^t
        for (int cnt = 0; cnt < epoch; ++cnt) {
            pow_b1_t *= beta_one;
            pow_b2_t *= beta_two;
        }

        // ---- per‑parameter updates --------------------------------
        for (int i = 0; i < SZ; ++i) {
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
        }
    }

    // ------------------------------------------------------------
    // 5. Output a small slice of the final parameters
    // ------------------------------------------------------------
    printf("First %d optimized parameters (epoch %d):\n", SZ < 10 ? SZ : 10, total_epochs);
    int j = 0;
    while (j < 10 && j < SZ) {
        printf("param[%d] = %.6f\n", j, param_vec[j]);
        ++j;
    }

    return 0;
}
