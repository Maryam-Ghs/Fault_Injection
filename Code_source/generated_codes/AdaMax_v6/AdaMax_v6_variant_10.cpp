/* LLM input variant 10: large-safe-stress */
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
    // Stress test problem:
    //   minimise f(x) = Σ_i (x_i - t_i)^2
    // Gradient:  df/dx_i = 2*(x_i - t_i)

    const int dimension = 1000;                     // larger problem dimension
    float* paramVec = new float[dimension];         // parameter vector on the heap
    float* target   = new float[dimension];         // target vector defining the optimum

    // Initialise parameters to zero and set a deterministic target pattern
    for (int i = 0; i < dimension; ++i) {
        paramVec[i] = 0.0f;
        // target_i cycles through values in [-0.4, 0.5] in steps of 0.1
        target[i] = ((i % 10) - 5) * 0.1f;  // e.g., -0.5, -0.4, ..., 0.4
    }

    // Create optimizer instance
    AdaMax optimizer(dimension);
    optimizer.maxIter = 2000;   // increased iteration count for convergence
    optimizer.alpha = 0.01f;    // slightly larger learning rate

    float* gradVec = new float[dimension]; // reusable gradient buffer

    // Main optimisation loop
    for (int itr = 0; itr < optimizer.maxIter; ++itr) {
        // compute gradient at current parameters
        for (int i = 0; i < dimension; ++i) {
            gradVec[i] = 2.0f * (paramVec[i] - target[i]);
        }

        // perform AdaMax step
        optimizer.stepUpdate(paramVec, gradVec);
    }

    // Output the final optimized parameters (showing first two for brevity)
    std::cout << "Optimized parameters after AdaMax:\n";
    std::cout << "w = " << paramVec[0] << "\n";
    std::cout << "v = " << paramVec[1] << "\n";

    delete[] gradVec;
    delete[] target;
    delete[] paramVec; // clean up
    return 0;
}
