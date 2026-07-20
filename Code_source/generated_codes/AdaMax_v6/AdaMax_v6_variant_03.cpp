/* LLM input variant 3: zeros-and-ones */
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
        alpha = 1.0f;          // identity learning rate
        b1 = 0.0f;             // zero decay (bias‑corrected first moment becomes gradient)
        b2 = 0.0f;             // zero decay for infinity norm
        eps = 1e-8f;
        maxIter = 0;           // empty-like case: no iterations
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
    // Simple test problem:
    //   minimise f(w, v) = (w - 3)^2 + (v + 2)^2
    // Gradient:  df/dw = 2*(w - 3),  df/dv = 2*(v + 2)

    int dimension = 2;                     // problem dimension
    float* paramVec = new float[dimension]; // parameter vector on the heap
    paramVec[0] = 0.0f; // initialise w with zero
    paramVec[1] = 1.0f; // initialise v with one

    // Create optimizer instance
    AdaMax optimizer(dimension);
    // optimizer.maxIter already set to 0 in constructor
    // optimizer.alpha already set to 1.0f in constructor

    // Main optimisation loop (will not run due to maxIter = 0)
    for (int itr = 0; itr < optimizer.maxIter; ++itr) {
        // compute gradient at current parameters
        float* gradVec = new float[dimension];
        gradVec[0] = 2.0f * (paramVec[0] - 3.0f); // ∂f/∂w
        gradVec[1] = 2.0f * (paramVec[1] + 2.0f); // ∂f/∂v

        // perform AdaMax step
        optimizer.stepUpdate(paramVec, gradVec);

        delete[] gradVec; // free temporary gradient array
    }

    // Output the final optimized parameters
    std::cout << "Optimized parameters after AdaMax:\n";
    std::cout << "w = " << paramVec[0] << "\n";
    std::cout << "v = " << paramVec[1] << "\n";

    delete[] paramVec; // clean up
    return 0;
}
