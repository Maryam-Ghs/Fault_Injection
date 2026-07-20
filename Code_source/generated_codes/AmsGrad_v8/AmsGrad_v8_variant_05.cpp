#include <iostream>
#include <vector>
#include <cmath>

/* LLM input variant 5: duplicate-heavy */

// ------------------------------------------------------------
// Helper: fill a vector with a repeating pattern of floats
// ------------------------------------------------------------
void fill_duplicate_pattern(std::vector<float>& target, int length)
{
    int idx = 0;
    while (idx < length)
    {
        // Pattern: 0.5, -0.5, 0.5, -0.5, ...
        target[idx] = (idx % 2 == 0) ? 0.5f : -0.5f;
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
        float one_minus_decay1 = 1.0f - decay1;
        m[j] = decay1 * m[j] + one_minus_decay1 * grads[j];

        float one_minus_decay2 = 1.0f - decay2;
        float grad_sq = grads[j] * grads[j];
        v[j] = decay2 * v[j] + one_minus_decay2 * grad_sq;

        if (v[j] > v_max[j])
            v_max[j] = v[j];

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
    // Configuration (all floats, no const, no unsigned)
    int vec_len = 10000;          // large array with many duplicates
    int epochs   = 5;            // number of passes over data
    float lr     = 0.01f;        // learning rate
    float beta1  = 0.9f;         // decay for first moment
    float beta2  = 0.999f;       // decay for second moment
    float epsilon = 1e-8f;       // small constant to avoid div‑0

    // Allocate vectors
    std::vector<float> param_vec(vec_len);
    std::vector<float> grad_vec(vec_len);
    std::vector<float> m_vec(vec_len);
    std::vector<float> v_vec(vec_len);
    std::vector<float> vmax_vec(vec_len);

    // Initialise all vectors with duplicate‑heavy data
    fill_duplicate_pattern(param_vec, vec_len);
    fill_duplicate_pattern(m_vec, vec_len);       // start from duplicated momenta
    fill_duplicate_pattern(v_vec, vec_len);
    fill_duplicate_pattern(vmax_vec, vec_len);
    // grads will be overwritten each epoch, no need to init

    // Optimization loop (verbose, step‑by‑step)
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

    // Final report (print first 10 parameters)
    std::cout << "\nFinal first 10 parameters after AmsGrad:\n";
    int out_idx = 0;
    while (out_idx < 10)
    {
        std::cout << param_vec[out_idx] << "\n";
        out_idx = out_idx + 1;
    }

    return 0;
}
