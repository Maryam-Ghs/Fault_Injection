#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>

/* LLM input variant 4: signed-extremes */

// ------------------------------------------------------------
// Helper: fill a vector with deterministic floats mixing negative, zero, and positive values
// ------------------------------------------------------------
void fill_random(std::vector<float>& target, int length)
{
    int idx = 0;
    while (idx < length)
    {
        // Deterministic pattern: -1, -0.5, 0, 0.5, 1, 0 (repeats)
        switch (idx % 6)
        {
            case 0: target[idx] = -1.0f; break;
            case 1: target[idx] = -0.5f; break;
            case 2: target[idx] =  0.0f; break;
            case 3: target[idx] =  0.5f; break;
            case 4: target[idx] =  1.0f; break;
            default: target[idx] =  0.0f; break;
        }
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
        // grad = 2 * param (reordered: (2.0f) * params[i])
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
        // m = decay1 * m + (1 - decay1) * grad
        float one_minus_decay1 = 1.0f - decay1;
        m[j] = decay1 * m[j] + one_minus_decay1 * grads[j];

        // ----- second moment (reordered) -----
        // v = decay2 * v + (1 - decay2) * grad^2
        float one_minus_decay2 = 1.0f - decay2;
        float grad_sq = grads[j] * grads[j];
        v[j] = decay2 * v[j] + one_minus_decay2 * grad_sq;

        // ----- maintain max of second moment -----
        // v_max = max(v_max, v)
        if (v[j] > v_max[j])
            v_max[j] = v[j];

        // ----- bias‑corrected first moment (reordered) -----
        // m_hat = m / (1 - decay1^t)   (t is implicit in step loop)
        // In this simple demo we skip bias correction for brevity

        // ----- parameter update (reordered) -----
        // params -= step_size * m / (sqrt(v_max) + eps)
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
    // Seed random generator (kept for completeness; not used)
    std::srand(static_cast<unsigned>(std::time(0)));

    // --------------------------------------------------------
    // Configuration (all floats, no const, no unsigned)
    // --------------------------------------------------------
    int vec_len = 10000;          // large deterministic array
    int epochs   = 5;             // number of passes over data
    float lr     = 0.01f;         // learning rate
    float beta1  = 0.9f;          // decay for first moment
    float beta2  = 0.999f;        // decay for second moment
    float epsilon = 1e-8f;        // small constant to avoid div‑0

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
    // Initialise all vectors with deterministic mixed signs
    // --------------------------------------------------------
    fill_random(param_vec, vec_len);   // mix of -1, -0.5, 0, 0.5, 1
    fill_random(m_vec, vec_len);      // same pattern for momentum

    // Initialise second‑moment vectors with non‑negative values to keep sqrt safe
    int i = 0;
    while (i < vec_len)
    {
        // Pattern: 0, 0.25, 0.5, 1 (repeats)
        switch (i % 4)
        {
            case 0: v_vec[i] = 0.0f; break;
            case 1: v_vec[i] = 0.25f; break;
            case 2: v_vec[i] = 0.5f; break;
            default: v_vec[i] = 1.0f; break;
        }
        // Offset pattern for vmax to vary slightly
        switch ((i + 1) % 4)
        {
            case 0: vmax_vec[i] = 0.0f; break;
            case 1: vmax_vec[i] = 0.25f; break;
            case 2: vmax_vec[i] = 0.5f; break;
            default: vmax_vec[i] = 1.0f; break;
        }
        i = i + 1;
    }
    // grads will be overwritten each epoch, no need to init

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
        while (sample < 5)
        {
            std::cout << param_vec[sample] << " ";
            sample = sample + 1;
        }
        std::cout << std::endl;

        epoch = epoch + 1;
    }

    // --------------------------------------------------------
    // Final report (print first 10 parameters)
    // --------------------------------------------------------
    std::cout << "\nFinal first 10 parameters after AmsGrad:\n";
    int out_idx = 0;
    while (out_idx < 10)
    {
        std::cout << param_vec[out_idx] << "\n";
        out_idx = out_idx + 1;
    }

    return 0;
}
