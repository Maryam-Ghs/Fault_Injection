#include <iostream>
#include <cmath>
#include <cstdint>

/* LLM input variant 9: medium-deterministic-random */

//---------------------------------------------------------------
// Compute gradient of 0.5 * ||w - t||^2  (t = target vector)
//---------------------------------------------------------------
void getGrad(float* w, float* t, float* g, int n)
{
    for (int i = 0; i < n; ++i)
        g[i] = w[i] - t[i];
}

//---------------------------------------------------------------
// Perform one Adagrad step
//---------------------------------------------------------------
void adagradStep(
    float* w,          // parameters (in/out)
    float* g,          // current gradient
    float* acc,        // accumulated squared grads (in/out)
    float eta,         // base learning rate
    float eps,         // epsilon for stability
    int n)
{
    // update accumulator first
    for (int i = 0; i < n; ++i)
        acc[i] += g[i] * g[i];

    // then update parameters using reordered arithmetic
    for (int i = 0; i < n; ++i)
    {
        float adj = eta / (std::sqrt(acc[i]) + eps);
        w[i] -= adj * g[i];
    }
}

// Simple deterministic pseudo‑random generator (LCG)
static uint32_t lcg_state = 123456789u;
inline uint32_t lcg_next()
{
    lcg_state = 1664525u * lcg_state + 1013904223u;
    return lcg_state;
}

// Map LCG output to a float in [low, high]
inline float lcg_float(float low, float high)
{
    return low + (static_cast<float>(lcg_next()) / static_cast<float>(UINT32_MAX)) * (high - low);
}

//---------------------------------------------------------------
// Main driver – everything lives here (heap allocated)
//---------------------------------------------------------------
int main()
{
    // deterministic problem size (medium)
    const int dim = 20;

    // learning parameters
    const float baseEta = 0.3f;
    const float epsilon = 1e-5f;
    const int epochs   = 25;

    // allocate vectors on the heap
    float* weight   = new float[dim];
    float* target   = new float[dim];
    float* grad     = new float[dim];
    float* accuGrad = new float[dim];

    // initialise weight and target with deterministic pseudo‑random values
    for (int i = 0; i < dim; ++i)
    {
        // weight in range [-3, 3]
        weight[i] = lcg_float(-3.0f, 3.0f);
        // target in range [0, 10] with a different offset for diversity
        target[i] = lcg_float(0.0f, 10.0f);
        grad[i] = 0.0f;
        accuGrad[i] = 0.0f;
    }

    // training loop
    for (int epoch = 0; epoch < epochs; ++epoch)
    {
        // compute current gradient
        getGrad(weight, target, grad, dim);

        // perform Adagrad update (order of operations swapped)
        adagradStep(weight, grad, accuGrad, baseEta, epsilon, dim);
    }

    // output final parameters
    std::cout << "Final weights after " << epochs << " epochs:\n";
    for (int i = 0; i < dim; ++i)
        std::cout << " w[" << i << "] = " << weight[i] << '\n';

    // clean up heap memory
    delete[] weight;
    delete[] target;
    delete[] grad;
    delete[] accuGrad;

    return 0;
}
