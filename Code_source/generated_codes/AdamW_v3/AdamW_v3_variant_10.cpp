#include <iostream>
#include <vector>
#include <cmath>

// LLM input variant 10: large-safe-stress

// -------------------------------------------------------------------
// Helper: fill a vector with given values (deterministic test data)
// -------------------------------------------------------------------
void fill_vector(std::vector<float>& target, const std::vector<float>& source)
{
    target.clear();
    for (size_t i = 0; i < source.size(); ++i)
    {
        target.push_back(source[i]);
    }
}

// -------------------------------------------------------------------
// Helper: perform one AdamW update step
// -------------------------------------------------------------------
void adamw_one_step(
    std::vector<float>& weights,            // parameters to be updated
    const std::vector<float>& grads,        // current gradient
    std::vector<float>& first_moment,       // m_t
    std::vector<float>& second_moment,      // v_t
    int step,                               // iteration number (starting from 1)
    float learning_rate,
    float beta_one,
    float beta_two,
    float epsilon,
    float weight_decay)
{
    // ---- 1. update biased first and second moments -----------------
    for (size_t idx = 0; idx < grads.size(); ++idx)
    {
        // m_t = beta1 * m_{t-1} + (1 - beta1) * g_t
        first_moment[idx] = beta_one * first_moment[idx] + (1.0f - beta_one) * grads[idx];

        // v_t = beta2 * v_{t-1} + (1 - beta2) * g_t^2
        float grad_sq = grads[idx] * grads[idx];
        second_moment[idx] = beta_two * second_moment[idx] + (1.0f - beta_two) * grad_sq;
    }

    // ---- 2. compute bias‑corrected moments -------------------------
    float bias_correction_one = 1.0f - std::pow(beta_one, static_cast<float>(step));
    float bias_correction_two = 1.0f - std::pow(beta_two, static_cast<float>(step));

    // ---- 3. apply AdamW update ------------------------------------
    for (size_t idx = 0; idx < weights.size(); ++idx)
    {
        // m̂_t = m_t / (1 - β1^t)
        float m_hat = first_moment[idx] / bias_correction_one;

        // v̂_t = v_t / (1 - β2^t)
        float v_hat = second_moment[idx] / bias_correction_two;

        // denominator = sqrt(v̂_t) + ε   (use sqrtf for float)
        float denom = std::sqrt(v_hat) + epsilon;

        // raw Adam step
        float step_size = learning_rate * m_hat / denom;

        // weight decay term (decoupled)
        float decay_term = learning_rate * weight_decay * weights[idx];

        // final parameter update (order: decay then Adam step)
        weights[idx] = weights[idx] - decay_term - step_size;
    }
}

// -------------------------------------------------------------------
// Main function: deterministic test of AdamW
// -------------------------------------------------------------------
int main()
{
    // ----- 0. hyper‑parameters (float only) -------------------------
    float lr      = 0.01f;      // learning rate
    float b1      = 0.9f;       // beta1
    float b2      = 0.999f;     // beta2
    float eps     = 1e-8f;      // epsilon
    float wd      = 0.01f;      // weight decay

    // ----- 1. deterministic large input size -----------------------
    const size_t INPUT_SIZE = 10000; // large but safe

    // ----- 2. deterministic initial weights -------------------------
    std::vector<float> w;                       // parameters
    std::vector<float> w_init(INPUT_SIZE);
    for (size_t i = 0; i < INPUT_SIZE; ++i)
    {
        // pattern: sinusoidal scaled values
        w_init[i] = std::sin(static_cast<float>(i)) * 0.5f;
    }
    fill_vector(w, w_init);

    // ----- 3. deterministic gradient (kept constant) -----------------
    std::vector<float> g;                       // gradient
    std::vector<float> g_fixed(INPUT_SIZE);
    for (size_t i = 0; i < INPUT_SIZE; ++i)
    {
        // pattern: repeating small values in [-0.2, 0.2]
        float base = static_cast<float>((i % 5) - 2) * 0.05f; // -0.1, -0.05, 0, 0.05, 0.1
        g_fixed[i] = base;
    }
    fill_vector(g, g_fixed);

    // ----- 4. moment vectors (initially zero) -----------------------
    std::vector<float> m(w.size(), 0.0f);        // first moment
    std::vector<float> v(w.size(), 0.0f);        // second moment

    // ----- 5. run a few AdamW steps ---------------------------------
    const int total_steps = 5;
    for (int s = 1; s <= total_steps; ++s)
    {
        std::cout << "=== Step " << s << " ===\n";

        // verbose: show first few moments before the update
        std::cout << "First moment (m) before (first 5): ";
        for (size_t i = 0; i < std::min<size_t>(5, m.size()); ++i) std::cout << m[i] << " ";
        std::cout << "\n";

        std::cout << "Second moment (v) before (first 5): ";
        for (size_t i = 0; i < std::min<size_t>(5, v.size()); ++i) std::cout << v[i] << " ";
        std::cout << "\n";

        // perform one AdamW update
        adamw_one_step(w, g, m, v, s, lr, b1, b2, eps, wd);

        // verbose: show first few moments after the update
        std::cout << "First moment (m) after (first 5) : ";
        for (size_t i = 0; i < std::min<size_t>(5, m.size()); ++i) std::cout << m[i] << " ";
        std::cout << "\n";

        std::cout << "Second moment (v) after (first 5): ";
        for (size_t i = 0; i < std::min<size_t>(5, v.size()); ++i) std::cout << v[i] << " ";
        std::cout << "\n";

        // verbose: show first few updated parameters
        std::cout << "Weights after update (first 5):   ";
        for (size_t i = 0; i < std::min<size_t>(5, w.size()); ++i) std::cout << w[i] << " ";
        std::cout << "\n\n";
    }

    // ----- 6. final result -------------------------------------------
    std::cout << "=== Final Weights (first 10) ===\n";
    for (size_t i = 0; i < std::min<size_t>(10, w.size()); ++i) std::cout << w[i] << " ";
    std::cout << "\n";

    return 0;
}
