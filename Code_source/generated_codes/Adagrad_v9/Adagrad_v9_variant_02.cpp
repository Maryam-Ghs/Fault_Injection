#include <iostream>
#include <cmath>

/* LLM input variant 2: small-diverse */

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
    int dim = 6;

    // learning parameters
    float baseEta = 0.3f;
    float epsilon = 1e-5f;
    int epochs   = 12;

    // allocate vectors on the heap
    float* weight   = new float[dim];
    float* target   = new float[dim];
    float* grad     = new float[dim];
    float* accuGrad = new float[dim];

    // initialise weight and target with varied fixed values
    for (int i = 0; i < dim; ++i)
    {
        weight[i]   = (i % 2 == 0) ? 1.0f : -1.0f;   // alternating start values
        // diverse target values: positive, negative, zero, fractional
        switch (i) {
            case 0: target[i] =  3.5f; break;
            case 1: target[i] = -1.2f; break;
            case 2: target[i] =  0.0f; break;
            case 3: target[i] =  5.5f; break;
            case 4: target[i] = -3.3f; break;
            case 5: target[i] =  2.2f; break;
        }
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
