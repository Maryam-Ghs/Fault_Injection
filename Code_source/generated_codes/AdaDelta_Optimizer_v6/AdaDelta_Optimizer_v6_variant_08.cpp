#include <iostream>
#include <cmath>

/* LLM input variant 8: sparse-skewed */

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
// Version #6 – AdaDelta optimizer demonstration
// ------------------------------------------------------------
int main()
{
    // larger problem with many zero entries (sparse, skewed)
    const int dims = 20;

    // initial weights (few non‑zeros, rest zero)
    float w[dims] = {
        1.0f, -2.0f, 0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f
    };

    // fake gradients for each iteration (mostly zero, few active entries)
    float grads[5][dims] = {
        // step 0: strong signal at index 0, weak at index 10
        { 0.3f, 0.0f, 0.0f, 0.0f, 0.0f,
          0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
          0.01f, 0.0f, 0.0f, 0.0f, 0.0f,
          0.0f, 0.0f, 0.0f, 0.0f, 0.0f },

        // step 1: activity shifts to index 1 and 12
        { 0.0f, 0.25f, 0.0f, 0.0f, 0.0f,
          0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
          0.0f, 0.0f, 0.008f, 0.0f, 0.0f,
          0.0f, 0.0f, 0.0f, 0.0f, 0.0f },

        // step 2: only index 2 active
        { 0.0f, 0.0f, 0.2f, 0.0f, 0.0f,
          0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
          0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
          0.0f, 0.0f, 0.0f, 0.0f, 0.0f },

        // step 3: sparse non‑zero at distant index 15
        { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
          0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
          0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
          0.005f, 0.0f, 0.0f, 0.0f, 0.0f },

        // step 4: all zeros (no gradient)
        { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
          0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
          0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
          0.0f, 0.0f, 0.0f, 0.0f, 0.0f }
    };

    // AdaDelta accumulators (initialized to zero)
    float eg[dims] = { 0.0f };
    float edx[dims] = { 0.0f };

    // hyper‑parameters
    float rho = 0.95f;
    float eps = 1e-6f;

    // run update steps
    int step = 0;
    while (step < 5)
    {
        adaDeltaStep(w, grads[step], eg, edx, dims, rho, eps);

        char label[32];
        sprintf(label, "After step %d: ", step + 1);
        showArray(label, w, dims);

        ++step;
    }

    std::cout << "Final parameters: ";
    showArray((char*)"", w, dims);

    return 0;
}
