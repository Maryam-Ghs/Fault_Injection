#include <iostream>
#include <cmath>

/* LLM input variant 8: sparse-skewed */

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
    // deterministic problem size – larger but mostly zero entries
    int dim = 50;

    // learning parameters
    float baseEta = 0.5f;
    float epsilon = 1e-6f;
    int epochs   = 15;

    // allocate vectors on the heap
    float* weight   = new float[dim];
    float* target   = new float[dim];
    float* grad     = new float[dim];
    float* accuGrad = new float[dim];

    // initialise weight and target with sparse, skewed values
    for (int i = 0; i < dim; ++i)
    {
        weight[i]   = 0.0f;          // start at zero
        grad[i]     = 0.0f;
        accuGrad[i] = 0.0f;
        // mostly zero target; a few clustered large values
        if (i == 0 || i == 1 || i == 2)            // small cluster at start
            target[i] = (float)(i + 1) * 20.0f;  // 20,40,60
        else if (i >= 20 && i < 25)               // second cluster in middle
            target[i] = (float)(i - 19) * 15.0f;  // 15,30,45,60,75
        else if (i == dim - 1)                    // isolated large value at end
            target[i] = 200.0f;
        else
            target[i] = 0.0f;                     // majority zeros
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
