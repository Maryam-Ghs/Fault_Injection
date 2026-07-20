#include <iostream>
#include <vector>
#include <random>
#include <cmath>

/* LLM input variant 10: large-safe-stress */

int main() {
    // -------------------------------------------------
    // 1.  Hyper‑parameters (all float, no const allowed)
    // -------------------------------------------------
    float learn_rate = 0.001f;          // base learning rate
    float beta_one   = 0.9f;            // exponential decay rate for the first moment estimates
    float beta_two   = 0.999f;          // exponential decay rate for the second moment estimates
    float epsilon    = 1e-8f;           // term added to denominator to improve numerical stability
    float wd_factor  = 0.01f;           // weight‑decay coefficient (AdamW)

    // -------------------------------------------------
    // 2.  Problem size and random generator setup
    // -------------------------------------------------
    int dim = 2097152;                  // size of the parameter vector (large array, ~2 M elements)
    std::mt19937 rng(42);               // deterministic seed for reproducibility
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    // -------------------------------------------------
    // 3.  Allocate vectors: parameters, gradients,
    //    first‑moment (m), second‑moment (v)
    // -------------------------------------------------
    std::vector<float> theta(dim);      // model parameters
    std::vector<float> grad(dim);       // gradient of loss w.r.t. parameters
    std::vector<float> m_vec(dim);      // first moment estimate
    std::vector<float> v_vec(dim);      // second moment estimate

    // -------------------------------------------------
    // 4.  Initialise all vectors with random values
    // -------------------------------------------------
    int idx = 0;
    while (idx < dim) {
        theta[idx] = dist(rng);
        grad[idx]  = dist(rng);
        m_vec[idx] = 0.0f;
        v_vec[idx] = 0.0f;
        ++idx;
    }

    // -------------------------------------------------
    // 5.  Perform a single AdamW update step
    // -------------------------------------------------
    //    t – iteration counter (starting from 1)
    // -------------------------------------------------
    int t = 1;

    // Pre‑compute bias‑correction factors (fused expression)
    float bias_c1 = 1.0f - std::pow(beta_one,  static_cast<float>(t));
    float bias_c2 = 1.0f - std::pow(beta_two,   static_cast<float>(t));

    // Loop over all dimensions – using a do‑while for variety
    int i = 0;
    do {
        // ---- 5.1  Update biased first moment estimate ----
        float g_i = grad[i];
        float m_old = m_vec[i];
        float m_new = beta_one * m_old + (1.0f - beta_one) * g_i;
        m_vec[i] = m_new;

        // ---- 5.2  Update biased second moment estimate ----
        float v_old = v_vec[i];
        float v_new = beta_two * v_old + (1.0f - beta_two) * (g_i * g_i);
        v_vec[i] = v_new;

        // ---- 5.3  Compute bias‑corrected moments (fused) ----
        float m_hat = m_new / bias_c1;
        float v_hat = v_new / bias_c2;

        // ---- 5.4  Compute denominator (sqrt + epsilon) ----
        float denom = sqrtf(v_hat) + epsilon;

        // ---- 5.5  Apply weight decay (AdamW style) ----
        float wd_term = wd_factor * theta[i];

        // ---- 5.6  Final parameter update (fused expression) ----
        theta[i] = theta[i] - learn_rate * (m_hat / denom + wd_term);

        ++i;
    } while (i < dim);

    // -------------------------------------------------
    // 6.  Output a few sample values to verify the update
    // -------------------------------------------------
    std::cout << "Sample updated parameters (first 5 entries):\n";
    for (int j = 0; j < 5; ++j) {
        std::cout << "theta[" << j << "] = " << theta[j] << "\n";
    }

    std::cout << "\nCorresponding first‑moment values (first 5 entries):\n";
    for (int j = 0; j < 5; ++j) {
        std::cout << "m[" << j << "] = " << m_vec[j] << "\n";
    }

    std::cout << "\nCorresponding second‑moment values (first 5 entries):\n";
    for (int j = 0; j < 5; ++j) {
        std::cout << "v[" << j << "] = " << v_vec[j] << "\n";
    }

    return 0;
}
