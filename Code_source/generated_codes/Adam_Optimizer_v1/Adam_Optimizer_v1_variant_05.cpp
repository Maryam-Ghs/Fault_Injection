#include <iostream>
#include <cmath>
#include <limits>

/* LLM input variant 5: duplicate-heavy */

//------------------------------------------------------------
// Simple utility to create a heap array of floats
//------------------------------------------------------------
float* createArray(int sz)
{
    return new float[sz];
}

//------------------------------------------------------------
// Fill an array with a heavy duplicate pattern: zeros, large values, and a negative constant
//------------------------------------------------------------
void initGradients(float* grad, int sz)
{
    for (int i = 0; i < sz; ++i)
    {
        switch (i % 3)
        {
            case 0: grad[i] = 0.0f;        // duplicate zero gradient
                    break;
            case 1: grad[i] = 1e6f;       // duplicate huge gradient
                    break;
            case 2: grad[i] = -3.14f;     // duplicate negative gradient
                    break;
        }
    }
}

//------------------------------------------------------------
// Initialize parameters with moderate values
//------------------------------------------------------------
void initParameters(float* param, int sz)
{
    for (int i = 0; i < sz; ++i)
        param[i] = 1.0f;
}

//------------------------------------------------------------
// Adam optimizer encapsulated in a class
//------------------------------------------------------------
class AdamOptimizer
{
public:
    AdamOptimizer(float lr, float b1, float b2, float eps, int dim)
        : learningRate(lr), beta1(b1), beta2(b2), epsilon(eps), size(dim)
    {
        // allocate first‑moment and second‑moment vectors
        m = createArray(size);
        v = createArray(size);
        // initialise to zero
        for (int i = 0; i < size; ++i)
        {
            m[i] = 0.0f;
            v[i] = 0.0f;
        }
        t = 0;
    }

    ~AdamOptimizer()
    {
        delete[] m;
        delete[] v;
    }

    // One optimisation step
    void step(float* param, const float* grad)
    {
        ++t;                                   // time step
        float oneMinusB1 = 1.0f - beta1;
        float oneMinusB2 = 1.0f - beta2;

        for (int i = 0; i < size; ++i)
        {
            // update biased first moment estimate
            m[i] = beta1 * m[i] + oneMinusB1 * grad[i];
            // update biased second moment estimate
            v[i] = beta2 * v[i] + oneMinusB2 * grad[i] * grad[i];
        }

        // bias‑corrected learning rates (reordered for readability)
        float alpha_t = learningRate * std::sqrt(1.0f - std::pow(beta2, (float)t))
                                    / (1.0f - std::pow(beta1, (float)t));

        for (int i = 0; i < size; ++i)
        {
            // compute denominator with epsilon first (mathematically safe)
            float denom = std::sqrt(v[i]) + epsilon;
            // final parameter update
            param[i] -= alpha_t * m[i] / denom;
        }
    }

private:
    float learningRate;
    float beta1;
    float beta2;
    float epsilon;
    int   size;
    int   t;          // iteration counter
    float* m;         // first moment vector
    float* v;         // second moment vector
};

//------------------------------------------------------------
// Main driver – generates data, runs optimizer, prints result
//------------------------------------------------------------
int main()
{
    const int dim = 12;                     // larger dimensionality to emphasize duplicates
    float* weights = createArray(dim);
    float* grads   = createArray(dim);

    initParameters(weights, dim);
    initGradients(grads, dim);

    // Adam hyper‑parameters (float literals only)
    float lr   = 0.01f;
    float b1   = 0.9f;
    float b2   = 0.999f;
    float eps  = 1e-8f;

    AdamOptimizer opt(lr, b1, b2, eps, dim);

    // Run a few steps to see the effect on duplicate‑heavy gradients
    const int steps = 3;
    for (int s = 0; s < steps; ++s)
    {
        opt.step(weights, grads);
    }

    // Output final parameters
    std::cout << "Final parameters after " << steps << " Adam steps:\n";
    for (int i = 0; i < dim; ++i)
    {
        std::cout << "w[" << i << "] = " << weights[i] << "\n";
    }

    delete[] weights;
    delete[] grads;
    return 0;
}
