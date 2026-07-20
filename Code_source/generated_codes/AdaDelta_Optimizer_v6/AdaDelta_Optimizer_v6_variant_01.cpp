/* LLM input variant 1: minimal-boundary */
#include <iostream>
#include <cmath>

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
    // minimal problem (1 parameter)
    int dims = 1;

    // initial weight
    float w[1] = { 1.0f };

    // single fake gradient
    float grads[1][1] = {
        { 0.2f }
    };

    // AdaDelta accumulators (initialized to zero)
    float eg[1] = { 0.0f };
    float edx[1] = { 0.0f };

    // hyper‑parameters (boundary value for rho)
    float rho = 1.0f;
    float eps = 1e-6f;

    // run one update step
    int step = 0;
    while (step < 1)
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
