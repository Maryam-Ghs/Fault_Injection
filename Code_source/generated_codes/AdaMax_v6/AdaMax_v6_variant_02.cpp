/* LLM input variant 2: small-diverse */
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
    // Small diverse test problem:
    //   minimise f(x, y, z) = (x + 1)^2 + (y - 3)^2 + (2*z + 1)^2
    // Gradient:  df/dx = 2*(x + 1)
    //            df/dy = 2*(y - 3)
    //            df/dz = 8*z + 4

    int dimension = 3;                     // problem dimension
    float* paramVec = new float[dimension]; // parameter vector on the heap
    paramVec[0] = 5.0f;   // initialise x
    paramVec[1] = -4.0f; // initialise y
    paramVec[2] = 1.5f;  // initialise z

    // Create optimizer instance
    AdaMax optimizer(dimension);
    optimizer.maxIter = 2000; // increased iterations for convergence
    optimizer.alpha = 0.02f;  // larger learning rate for faster progress

    // Main optimisation loop
    for (int itr = 0; itr < optimizer.maxIter; ++itr) {
        // compute gradient at current parameters
        float* gradVec = new float[dimension];
        gradVec[0] = 2.0f * (paramVec[0] + 1.0f);          // ∂f/∂x
        gradVec[1] = 2.0f * (paramVec[1] - 3.0f);          // ∂f/∂y
        gradVec[2] = 8.0f * paramVec[2] + 4.0f;            // ∂f/∂z

        // perform AdaMax step
        optimizer.stepUpdate(paramVec, gradVec);

        delete[] gradVec; // free temporary gradient array
    }

    // Output the final optimized parameters
    std::cout << "Optimized parameters after AdaMax:\n";
    std::cout << "x = " << paramVec[0] << "\n";
    std::cout << "y = " << paramVec[1] << "\n";
    std::cout << "z = " << paramVec[2] << "\n";

    delete[] paramVec; // clean up
    return 0;
}
