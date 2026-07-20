// LLM input variant 6: ordered-structured
// AdaMax optimizer implementation – version #6
#include <iostream>
#include <cmath>

class AdaMax {
public:
    float alpha;      // learning rate
    float b1;         // exponential decay rate for the first moment estimates
    float b2;         // exponential decay rate for the infinity norm
    float eps;        // small constant to avoid division by zero
    int maxIter;      // number of optimization steps
    int dim;          // dimension of the parameter vector
    float* m;         // first moment vector (heap allocated)
    float* u;         // infinity norm vector (heap allocated)
    int step;         // current iteration count

    // Constructor: allocate memory and initialise members
    AdaMax(int d) {
        dim = d;
        alpha = 0.002f;
        b1 = 0.9f;
        b2 = 0.999f;
        eps = 1e-8f;
        maxIter = 1000;
        step = 0;
        m = new float[dim];
        u = new float[dim];
        for (int i = 0; i < dim; ++i) {
            m[i] = 0.0f;
            u[i] = 0.0f;
        }
    }

    // Destructor: free heap memory
    ~AdaMax() {
        delete[] m;
        delete[] u;
    }

    // Perform one AdaMax update step
    void stepUpdate(float* param, float* grad) {
        ++step;
        float biasCorr = 1.0f - powf(b1, (float)step);
        for (int i = 0; i < dim; ++i) {
            // update biased first moment estimate
            m[i] = b1 * m[i] + (1.0f - b1) * grad[i];

            // update exponentially weighted infinity norm
            float absGrad = grad[i] > 0.0f ? grad[i] : -grad[i];
            float scaledPrev = b2 * u[i];
            u[i] = (scaledPrev > absGrad) ? scaledPrev : absGrad;

            // parameter update (bias‑corrected learning rate applied)
            param[i] = param[i] - (alpha / biasCorr) * (m[i] / (u[i] + eps));
        }
    }
};

int main() {
    // Structured test problem:
    //   minimise f(x) = Σ_i (x_i - t_i)^2  where t_i = i + 1 (ordered targets)
    // Gradient:  df/dx_i = 2*(x_i - t_i)

    const int dimension = 10;                     // problem dimension (ordered structure)
    float* paramVec = new float[dimension];      // parameter vector on the heap
    float* targetVec = new float[dimension];     // ordered target values

    // Initialise parameters in a sorted descending order and targets in ascending order
    for (int i = 0; i < dimension; ++i) {
        paramVec[i] = -static_cast<float>(i) * 0.5f;   // e.g., 0.0, -0.5, -1.0, ...
        targetVec[i] = static_cast<float>(i + 1);    // 1.0, 2.0, 3.0, ...
    }

    // Create optimizer instance
    AdaMax optimizer(dimension);
    optimizer.maxIter = 5000; // run more iterations for convergence
    optimizer.alpha = 0.01f;  // a slightly larger learning rate

    // Main optimisation loop
    for (int itr = 0; itr < optimizer.maxIter; ++itr) {
        // compute gradient at current parameters
        float* gradVec = new float[dimension];
        for (int i = 0; i < dimension; ++i) {
            gradVec[i] = 2.0f * (paramVec[i] - targetVec[i]);
        }

        // perform AdaMax step
        optimizer.stepUpdate(paramVec, gradVec);

        delete[] gradVec; // free temporary gradient array
    }

    // Output the final optimized parameters
    std::cout << "Optimized parameters after AdaMax:\n";
    for (int i = 0; i < dimension; ++i) {
        std::cout << "x[" << i << "] = " << paramVec[i] << "\n";
    }

    delete[] paramVec;   // clean up
    delete[] targetVec;  // clean up
    return 0;
}
