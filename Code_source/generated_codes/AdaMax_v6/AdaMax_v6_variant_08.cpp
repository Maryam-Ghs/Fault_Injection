/* LLM input variant 8: sparse-skewed */
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
    // Sparse-skewed test problem:
    //   minimise f(x) = Σ_i (x_i - t_i)^2
    // where t_i (target) is zero for the vast majority of indices,
    // and non‑zero only for a few clustered positions.
    const int dimension = 1000;                     // high‑dimensional problem
    float* paramVec = new float[dimension];         // parameter vector on the heap
    float* target = new float[dimension];            // sparse target vector

    // Initialise parameters and target
    for (int i = 0; i < dimension; ++i) {
        paramVec[i] = 0.0f;
        target[i] = 0.0f;
    }
    // Define a few non‑zero target entries (clustered around index 500)
    target[498] = 1.5f;
    target[499] = -2.0f;
    target[500] = 3.0f;
    target[501] = -4.5f;
    target[502] = 2.5f;

    // Create optimizer instance
    AdaMax optimizer(dimension);
    optimizer.maxIter = 3000; // sufficient iterations for convergence
    optimizer.alpha = 0.01f; // a slightly larger learning rate

    // Main optimisation loop
    for (int itr = 0; itr < optimizer.maxIter; ++itr) {
        // compute gradient at current parameters (sparse‑skewed)
        float* gradVec = new float[dimension];
        for (int i = 0; i < dimension; ++i) {
            // gradient of (x_i - t_i)^2 is 2*(x_i - t_i)
            gradVec[i] = 2.0f * (paramVec[i] - target[i]);
        }

        // perform AdaMax step
        optimizer.stepUpdate(paramVec, gradVec);

        delete[] gradVec; // free temporary gradient array
    }

    // Output a subset of the final optimized parameters (non‑zero targets)
    std::cout << "Optimized parameters for sparse indices after AdaMax:\n";
    for (int i = 498; i <= 502; ++i) {
        std::cout << "x[" << i << "] = " << paramVec[i] << "\n";
    }

    delete[] paramVec;
    delete[] target;
    return 0;
}
