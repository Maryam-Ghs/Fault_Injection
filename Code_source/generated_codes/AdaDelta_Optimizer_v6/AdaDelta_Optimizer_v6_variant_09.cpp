#include <iostream>
#include <cmath>

/* LLM input variant 9: medium-deterministic-random */

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
    // medium‑sized predefined problem (7 parameters)
    int dims = 7;

    // initial weights
    float w[7] = { 1.0f, -2.0f, 0.5f, -0.3f, 0.8f, -1.2f, 0.0f };

    // deterministic pseudo‑random gradients for each iteration
    float grads[7][7] = {
        { 0.21f, -0.11f, 0.06f, -0.02f, 0.03f, -0.01f, 0.005f },
        { 0.19f, -0.09f, 0.055f, -0.018f, 0.028f, -0.009f, 0.0045f },
        { 0.17f, -0.07f, 0.05f, -0.015f, 0.025f, -0.008f, 0.004f },
        { 0.15f, -0.06f, 0.045f, -0.013f, 0.022f, -0.007f, 0.0035f },
        { 0.13f, -0.05f, 0.04f, -0.011f, 0.019f, -0.006f, 0.003f },
        { 0.11f, -0.04f, 0.035f, -0.009f, 0.016f, -0.005f, 0.0025f },
        { 0.09f, -0.03f, 0.03f, -0.007f, 0.013f, -0.004f, 0.002f }
    };

    // AdaDelta accumulators (initialized to zero)
    float eg[7] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
    float edx[7] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

    // hyper‑parameters
    float rho = 0.95f;
    float eps = 1e-6f;

    // run a few update steps
    int step = 0;
    while (step < 7)
    {
        // perform one AdaDelta update using the current gradient
        adaDeltaStep(w, grads[step], eg, edx, dims, rho, eps);

        // show intermediate result
        char label[32];
        sprintf(label, "After step %d: ", step + 1);
        showArray(label, w, dims);

        ++step;
    }

    // final outcome
    std::cout << "Final parameters: ";
    showArray((char*)"", w, dims);

    return 0;
}
