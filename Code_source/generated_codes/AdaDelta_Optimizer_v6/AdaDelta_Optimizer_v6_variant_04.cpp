/* LLM input variant 4: signed-extremes */
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
    // small predefined problem (4 parameters)
    int dims = 4;

    // initial weights (mix of negative, zero, positive)
    float w[4] = { -10.0f, 0.0f, 5.0f, -0.5f };

    // fake gradients for each iteration (mix of negative, zero, positive)
    float grads[5][4] = {
        { -0.30f,  0.0f,  0.20f, -0.10f },
        {  0.15f, -0.05f, 0.00f,  0.05f },
        { -0.10f,  0.10f, -0.20f, 0.00f },
        {  0.07f, -0.07f, 0.03f, -0.02f },
        { -0.05f,  0.00f, 0.01f,  0.04f }
    };

    // AdaDelta accumulators (initialized to zero)
    float eg[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    float edx[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

    // hyper‑parameters
    float rho = 0.95f;
    float eps = 1e-6f;

    // run a few update steps
    int step = 0;
    while (step < 5)
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
