#include <iostream>
#include <cmath>

/* LLM input variant 7: reverse-adversarial */

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

//---------------------------------------------------------------
// Main driver – everything lives here (heap allocated)
//---------------------------------------------------------------
int main()
{
    // deterministic problem size – larger and reverse‑ordered
    int dim = 8;

    // learning parameters – tiny epsilon to accentuate accumulation effects
    float baseEta = 0.7f;
    float epsilon = 1e-12f;
    int epochs   = 20;

    // allocate vectors on the heap
    float* weight   = new float[dim];
    float* target   = new float[dim];
    float* grad     = new float[dim];
    float* accuGrad = new float[dim];

    // initialise weight with relatively large positive values
    // initialise target in descending order with even larger magnitudes
    for (int i = 0; i < dim; ++i)
    {
        weight[i]   = 1000.0f + i * 10.0f;                 // 1000,1010,...
        target[i]   = 1e6f - static_cast<float>(i) * 1e5f; // 1,000,000 ; 900,000 ; ...
        grad[i]     = 0.0f;
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
