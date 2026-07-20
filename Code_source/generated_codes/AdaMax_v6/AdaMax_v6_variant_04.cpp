// LLM input variant 4: signed-extremes
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
    // Simple test problem:
    //   minimise f(w, v) = (w - 3)^2 + (v + 2)^2
    // Gradient:  df/dw = 2*(w - 3),  df/dv = 2*(v + 2)

    int dimension = 2;                     // problem dimension
    float* paramVec = new float[dimension]; // parameter vector on the heap
    paramVec[0] = -5.0f; // initialise w with a negative value
    paramVec[1] = 7.0f;  // initialise v with a positive value

    // Create optimizer instance
    AdaMax optimizer(dimension);
    optimizer.maxIter = 3000; // increased iterations
    optimizer.alpha = 0.02f;   // larger learning rate
    optimizer.b1 = 0.5f;       // moderate decay rate for first moment

    // Main optimisation loop
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
