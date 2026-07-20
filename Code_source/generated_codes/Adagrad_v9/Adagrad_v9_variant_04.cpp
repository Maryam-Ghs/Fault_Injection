#include <iostream>
#include <cmath>

/* LLM input variant 4: signed-extremes */

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
    // deterministic problem size
    int dim = 5;

    // learning parameters
    float baseEta = 0.5f;
    float epsilon = 1e-6f;
    int epochs   = 15;

    // allocate vectors on the heap
    float* weight   = new float[dim];
    float* target   = new float[dim];
    float* grad     = new float[dim];
    float* accuGrad = new float[dim];

    // initialise weight and target with a mix of negative, zero, and positive values
    // weight: -1, 0, -3, 2, -5
    // target: 10, -5, 0, 7.5, -2
    float initWeight[5] = { -1.0f, 0.0f, -3.0f, 2.0f, -5.0f };
    float initTarget[5] = { 10.0f, -5.0f, 0.0f, 7.5f, -2.0f };
    for (int i = 0; i < dim; ++i)
    {
        weight[i]   = initWeight[i];
        target[i]   = initTarget[i];
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
