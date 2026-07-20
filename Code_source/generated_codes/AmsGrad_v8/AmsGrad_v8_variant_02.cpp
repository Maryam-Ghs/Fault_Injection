#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>

// LLM input variant 2: small-diverse

// ------------------------------------------------------------
// Helper: fill a vector with deterministic diverse floats
// ------------------------------------------------------------
void fill_deterministic(std::vector<float>& target, int length)
{
    int idx = 0;
    while (idx < length)
    {
        // Create a varied pattern: -0.8, -0.4, 0.0, 0.4, 0.8, -0.6, 0.2, -0.2, ...
        float base = static_cast<float>((idx % 5) - 2) * 0.4f;
        // Add a small offset based on index parity
        if (idx % 2 == 0)
            target[idx] = base;
        else
            target[idx] = base + 0.1f;
        idx = idx + 1;
    }
}

// ------------------------------------------------------------
// Helper: compute a synthetic gradient (here 2 * param)
// ------------------------------------------------------------
void compute_gradients(const std::vector<float>& params,
                       std::vector<float>& grads,
                       int length)
{
    int i = 0;
    while (i < length)
    {
        grads[i] = 2.0f * params[i];
        i = i + 1;
    }
}

// ------------------------------------------------------------
// Helper: a single AmsGrad update step
// ------------------------------------------------------------
void amsgrad_update(std::vector<float>& params,
                    const std::vector<float>& grads,
                    std::vector<float>& m,
                    std::vector<float>& v,
                    std::vector<float>& v_max,
                    int length,
                    float step_size,
                    float decay1,
                    float decay2,
                    float eps)
{
    int j = 0;
    while (j < length)
    {
        // ----- first moment (reordered) -----
        float one_minus_decay1 = 1.0f - decay1;
        m[j] = decay1 * m[j] + one_minus_decay1 * grads[j];

        // ----- second moment (reordered) -----
        float one_minus_decay2 = 1.0f - decay2;
        float grad_sq = grads[j] * grads[j];
        v[j] = decay2 * v[j] + one_minus_decay2 * grad_sq;

        // ----- maintain max of second moment -----
        if (v[j] > v_max[j])
            v_max[j] = v[j];

        // ----- parameter update (reordered) -----
        float denom = std::sqrt(v_max[j]) + eps;
        float update = step_size * m[j] / denom;
        params[j] = params[j] - update;

        j = j + 1;
    }
}

// ------------------------------------------------------------
// Main driver
// ------------------------------------------------------------
int main()
{
    // --------------------------------------------------------
    // Configuration (all floats, no const, no unsigned)
    // --------------------------------------------------------
    int vec_len = 8;            // small diverse array
    int epochs   = 3;           // fewer passes
    float lr     = 0.05f;       // learning rate
    float beta1  = 0.85f;       // decay for first moment
    float beta2  = 0.95f;       // decay for second moment
    float epsilon = 1e-7f;      // small constant to avoid div‑0

    // --------------------------------------------------------
    // Allocate vectors
    // --------------------------------------------------------
    std::vector<float> param_vec;
    std::vector<float> grad_vec;
    std::vector<float> m_vec;
    std::vector<float> v_vec;
    std::vector<float> vmax_vec;

    param_vec.resize(vec_len);
    grad_vec.resize(vec_len);
    m_vec.resize(vec_len);
    v_vec.resize(vec_len);
    vmax_vec.resize(vec_len);

    // --------------------------------------------------------
    // Initialise all vectors with deterministic diverse values
    // --------------------------------------------------------
    fill_deterministic(param_vec, vec_len);
    fill_deterministic(m_vec, vec_len);
    fill_deterministic(v_vec, vec_len);
    fill_deterministic(vmax_vec, vec_len);
    // grads will be overwritten each epoch

    // --------------------------------------------------------
    // Optimization loop (verbose, step‑by‑step)
    // --------------------------------------------------------
    int epoch = 0;
    while (epoch < epochs)
    {
        // 1. Compute current gradients
        compute_gradients(param_vec, grad_vec, vec_len);

        // 2. Apply AmsGrad update
        amsgrad_update(param_vec,
                       grad_vec,
                       m_vec,
                       v_vec,
                       vmax_vec,
                       vec_len,
                       lr,
                       beta1,
                       beta2,
                       epsilon);

        // 3. Print a tiny snapshot for verification
        std::cout << "Epoch " << epoch + 1 << " sample params: ";
        int sample = 0;
        while (sample < 5 && sample < vec_len)
        {
            std::cout << param_vec[sample] << " ";
            sample = sample + 1;
        }
        std::cout << std::endl;

        epoch = epoch + 1;
    }

    // --------------------------------------------------------
    // Final report (print all parameters)
    // --------------------------------------------------------
    std::cout << "\nFinal parameters after AmsGrad:\n";
    int out_idx = 0;
    while (out_idx < vec_len)
    {
        std::cout << param_vec[out_idx] << "\n";
        out_idx = out_idx + 1;
    }

    return 0;
}
