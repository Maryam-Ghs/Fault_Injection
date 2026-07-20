#include <iostream>
#include <cmath>
#include <limits>

/* LLM input variant 2: small-diverse */

//------------------------------------------------------------
// Simple utility to create a heap array of floats
//------------------------------------------------------------
float* createArray(int sz)
{
    return new float[sz];
}

//------------------------------------------------------------
// Fill an array with a repeating pattern of varied ordinary values
//------------------------------------------------------------
void initGradients(float* grad, int sz)
{
    for (int i = 0; i < sz; ++i)
    {
        switch (i % 6)
        {
            case 0: grad[i] = 0.0f;                     // zero
                    break;
            case 1: grad[i] = 2.5f;                     // modest positive
                    break;
            case 2: grad[i] = -2.5f;                    // modest negative
                    break;
            case 3: grad[i] = 1e5f;                    // large positive
                    break;
            case 4: grad[i] = 1e-5f;                   // small positive
                    break;
            case 5: grad[i] = std::numeric_limits<float>::infinity(); // infinite
                    break;
        }
    }
}

//------------------------------------------------------------
// Initialize parameters with modest values
//------------------------------------------------------------
void initParameters(float* param, int sz)
{
    for (int i = 0; i < sz; ++i)
        param[i] = 0.5f;
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
        m = createArray(size);
        v = createArray(size);
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
        ++t;
        float oneMinusB1 = 1.0f - beta1;
        float oneMinusB2 = 1.0f - beta2;

        for (int i = 0; i < size; ++i)
        {
            m[i] = beta1 * m[i] + oneMinusB1 * grad[i];
            v[i] = beta2 * v[i] + oneMinusB2 * grad[i] * grad[i];
        }

        float alpha_t = learningRate * std::sqrt(1.0f - std::pow(beta2, (float)t))
                                    / (1.0f - std::pow(beta1, (float)t));

        for (int i = 0; i < size; ++i)
        {
            float denom = std::sqrt(v[i]) + epsilon;
            param[i] -= alpha_t * m[i] / denom;
        }
    }

private:
    float learningRate;
    float beta1;
    float beta2;
    float epsilon;
    int   size;
    int   t;
    float* m;
    float* v;
};

//------------------------------------------------------------
// Main driver – generates data, runs optimizer, prints result
//------------------------------------------------------------
int main()
{
    const int dim = 7;                      // diversified small dimensionality
    float* weights = createArray(dim);
    float* grads   = createArray(dim);

    initParameters(weights, dim);
    initGradients(grads, dim);

    // Adam hyper‑parameters (float literals only)
    float lr   = 0.005f;
    float b1   = 0.85f;
    float b2   = 0.95f;
    float eps  = 1e-7f;

    AdamOptimizer opt(lr, b1, b2, eps, dim);

    // Run a few steps to see the effect on varied gradients
    const int steps = 4;
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
