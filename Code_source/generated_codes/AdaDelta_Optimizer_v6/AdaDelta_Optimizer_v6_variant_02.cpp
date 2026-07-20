#include <iostream>
#include <cmath>

/* LLM input variant 2: small-diverse */

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
    // small predefined problem (4 parameters)
    int dims = 4;

    // initial weights
    float w[4] = { 2.5f, -1.5f, 0.0f, 3.3f };

    // fake gradients for each iteration (could be from a loss)
    float grads[4][4] = {
        { 0.30f, -0.12f, 0.07f, -0.05f },
        { 0.25f, -0.10f, 0.06f, -0.04f },
        { 0.20f, -0.08f, 0.05f, -0.03f },
        { 0.15f, -0.06f, 0.04f, -0.02f }
    };

    // AdaDelta accumulators (initialized to zero)
    float eg[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    float edx[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

    // hyper‑parameters
    float rho = 0.90f;
    float eps = 1e-5f;

    // run a few update steps
    int step = 0;
    while (step < 4)
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
