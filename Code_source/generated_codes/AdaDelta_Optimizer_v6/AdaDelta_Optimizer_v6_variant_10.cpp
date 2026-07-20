#include <iostream>
#include <cmath>
#include <cstdio>

/* LLM input variant 10: large-safe-stress */

// ------------------------------------------------------------
// AdaDelta helper: one update step for a vector of parameters
// ------------------------------------------------------------
void adaDeltaStep(float* weight, float* grad, float* accGrad,
                  float* accUpd, int size, float rho, float eps)
{
    int idx = 0;
    while (idx < size)
    {
        // accumulate squared gradients: Eg = rho*Eg + (1-rho)*g^2
        accGrad[idx] = rho * accGrad[idx] + (1.0f - rho) * grad[idx] * grad[idx];

        // compute the adaptive update:
        //   delta = (sqrt(Edx+eps) / sqrt(Eg+eps)) * g
        // reordered as: delta = (g * sqrt(Edx+eps)) / sqrt(Eg+eps)
        float num = grad[idx] * sqrtf(accUpd[idx] + eps);
        float den = sqrtf(accGrad[idx] + eps);
        float delta = num / den;

        // accumulate squared updates: Edx = rho*Edx + (1-rho)*delta^2
        accUpd[idx] = rho * accUpd[idx] + (1.0f - rho) * delta * delta;

        // apply the update
        weight[idx] = weight[idx] - delta;

        ++idx;
    }
}

// ------------------------------------------------------------
// Simple printer for a float array (stack allocated)
// ------------------------------------------------------------
void showArray(char* title, float* arr, int size)
{
    std::cout << title;
    int i = 0;
    while (i < size)
    {
        std::cout << arr[i];
        if (i + 1 < size) std::cout << ", ";
        ++i;
    }
    std::cout << '\n';
}

// ------------------------------------------------------------
// Version #6 – AdaDelta optimizer demonstration (large‑safe‑stress)
// ------------------------------------------------------------
int main()
{
    // larger predefined problem (1000 parameters)
    const int dims = 1000;

    // initial weights (linearly spaced for variety)
    float w[dims];
    for (int i = 0; i < dims; ++i)
        w[i] = 1.0f - 0.001f * i;  // values from 1.0 down to ~0.0

    // number of update steps
    const int steps = 10;

    // fake gradients for each iteration (gradients decay over steps)
    float grads[steps][dims];
    for (int s = 0; s < steps; ++s)
    {
        float factor = 0.2f / (s + 1);  // decreasing magnitude
        for (int i = 0; i < dims; ++i)
        {
            // alternating sign pattern with small magnitude
            grads[s][i] = ((i % 2 == 0) ? 1.0f : -1.0f) * factor * (1.0f + 0.0001f * i);
        }
    }

    // AdaDelta accumulators (initialized to zero)
    float eg[dims] = {0.0f};
    float edx[dims] = {0.0f};

    // hyper‑parameters
    const float rho = 0.95f;
    const float eps = 1e-6f;

    // run update steps
    int step = 0;
    while (step < steps)
    {
        adaDeltaStep(w, grads[step], eg, edx, dims, rho, eps);

        char label[64];
        std::snprintf(label, sizeof(label), "After step %d: ", step + 1);
        showArray(label, w, dims);

        ++step;
    }

    // final outcome
    std::cout << "Final parameters: ";
    showArray((char*)"", w, dims);

    return 0;
}
