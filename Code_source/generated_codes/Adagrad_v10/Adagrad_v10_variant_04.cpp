/* LLM input variant 4: signed-extremes */
 // Version 10 - Adagrad implementation (branch‑minimized, vector‑based, class‑oriented)

#include <iostream>
#include <vector>
#include <cmath>

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
    // Edge‑case heavy synthetic problem:
    // Minimize  f(w) = Σ (w_i - target_i)^2
    // Gradient: g_i = 2 * (w_i - target_i)

    int dim = 7;                         // dimensionality
    std::vector<float> weights(dim, 0.0f); // start at zero
    std::vector<float> target = { -5.0f, 0.0f, 3.5f, -2.5f, 0.0f, 8.0f, -9.0f };

    // Learning rate and epsilon chosen to stress the algorithm
    float lr   = 0.5f;      // moderate step size
    float eps  = 1e-6f;     // tiny epsilon

    AdaGradOpt optimizer(dim, lr, eps);

    // Run many iterations to hit edge cases (zero gradients, large accumulations)
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
