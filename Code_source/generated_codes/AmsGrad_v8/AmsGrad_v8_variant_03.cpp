#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>

/* LLM input variant 3: zeros-and-ones */

// ------------------------------------------------------------
// Helper: fill a vector with random floats in range [-1, 1]
// ------------------------------------------------------------
void fill_random(std::vector<float>& target, int length)
{
    int idx = 0;
    while (idx < length)
    {
        // rand() returns int, cast to float and scale to [-1, 1]
        float rnd = (static_cast<float>(std::rand()) / RAND_MAX) * 2.0f - 1.0f;
        target[idx] = rnd;
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
    // Seed random generator (kept for completeness)
    std::srand(static_cast<unsigned>(std::time(0)));

    // --------------------------------------------------------
    // Configuration (all floats, no const, no unsigned)
    // --------------------------------------------------------
    int vec_len = 10000;          // large array
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
    // Initialise all vectors with zeros and ones (identity-like)
    // --------------------------------------------------------
    for (int i = 0; i < vec_len; ++i) {
        param_vec[i] = 0.0f;   // all parameters start at zero
        m_vec[i]     = 1.0f;   // first moment initialized to one
        v_vec[i]     = 0.0f;   // second moment starts at zero
        vmax_vec[i]  = 1.0f;   // max second moment starts at one
    }
    // grads will be computed each epoch; no need to pre‑fill

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
