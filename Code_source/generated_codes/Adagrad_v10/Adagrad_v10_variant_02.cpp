// Version 10 - Adagrad implementation (branch‑minimized, vector‑based, class‑oriented)

#include <iostream>
#include <vector>
#include <cmath>

/* LLM input variant 2: small-diverse */

class AdaGradOpt {
public:
    float stepSize;          // learning rate
    float smooth;            // epsilon to avoid division by zero
    std::vector<float> accGradSq; // accumulated squared gradients

    // Constructor: dimension, learning rate, epsilon
    AdaGradOpt(int dim, float lr, float eps) {
        stepSize = lr;
        smooth   = eps;
        accGradSq.assign(dim, 0.0f);
    }

    // Perform one update step
    void step(std::vector<float>& weights, const std::vector<float>& dW) {
        int idx = 0;
        while (idx < (int)weights.size()) {
            // accumulate squared gradient
            accGradSq[idx] = accGradSq[idx] + dW[idx] * dW[idx];
            // compute adapted learning rate (sqrt is sqrtf → float)
            float adapt = stepSize / (sqrtf(accGradSq[idx]) + smooth);
            // update weight
            weights[idx] = weights[idx] - adapt * dW[idx];
            ++idx;
        }
    }
};

int main() {
    // Small diverse synthetic problem:
    // Minimize  f(w) = Σ (w_i - target_i)^2
    // Gradient: g_i = 2 * (w_i - target_i)

    int dim = 7;                         // dimensionality
    std::vector<float> weights = {0.5f, -0.3f, 2.1f, 0.0f, -1.5f, 3.3f, -2.2f};
    std::vector<float> target = {1.2f, -1.8f, 3.5f, 0.0f, 5.0f, -0.7f, 2.2f};

    // Learning rate and epsilon chosen for varied behavior
    float lr   = 0.5f;      // moderate step size
    float eps  = 1e-5f;     // small epsilon

    AdaGradOpt optimizer(dim, lr, eps);

    // Run a moderate number of iterations
    int iter = 0;
    while (iter < 150) {
        // compute gradient
        std::vector<float> grad(dim, 0.0f);
        int i = 0;
        while (i < dim) {
            grad[i] = 2.0f * (weights[i] - target[i]);
            ++i;
        }

        // perform Adagrad update
        optimizer.step(weights, grad);

        ++iter;
    }

    // Print final weights
    std::cout << "Final weights after " << iter << " steps:\n";
    for (int i = 0; i < dim; ++i) {
        std::cout << "w[" << i << "] = " << weights[i] << '\n';
    }

    // Edge‑case verification: print accumulated squares for inspection
    std::cout << "\nAccumulated squared gradients:\n";
    for (int i = 0; i < dim; ++i) {
        std::cout << "acc[" << i << "] = " << optimizer.accGradSq[i] << '\n';
    }

    return 0;
}
