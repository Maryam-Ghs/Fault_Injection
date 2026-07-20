/******************************************************************
 * AmsGrad Optimizer – Version 10
 * --------------------------------------------------------------
 * Implements the AmsGrad algorithm using only float and int types.
 * All data resides on the stack; the implementation is class‑based,
 * loop‑heavy, and uses fused arithmetic expressions.
 */
/* LLM input variant 10: large-safe-stress */

#include <iostream>
#include <cstdlib>
#include <cmath>

#define MAX_N 1024          // maximum size of parameter vector (stack allocation)

class AmsGrad {
public:
    int dim;                // actual dimension used (<= MAX_N)
    float rate;            // learning rate
    float beta_one;        // exponential decay rate for the first moment
    float beta_two;        // exponential decay rate for the second moment
    float epsilon;         // small constant to avoid division by zero

    // Stack‑allocated state vectors
    float theta[MAX_N];
    float moment[MAX_N];
    float var_acc[MAX_N];
    float var_max[MAX_N];

    // Constructor – initializes all buffers to zero
    AmsGrad(int d, float lr, float b1, float b2, float eps) {
        dim = d;
        rate = lr;
        beta_one = b1;
        beta_two = b2;
        epsilon = eps;

        int i = 0;
        while (i < dim) {
            theta[i] = 0.0f;
            moment[i] = 0.0f;
            var_acc[i] = 0.0f;
            var_max[i] = 0.0f;
            i = i + 1;
        }
    }

    // One optimization step given a gradient vector
    void step(float *grad) {
        int i = 0;
        while (i < dim) {
            // Update biased first moment estimate
            moment[i] = beta_one * moment[i] + (1.0f - beta_one) * grad[i];

            // Compute squared gradient once (fused)
            float g_sq = grad[i] * grad[i];

            // Update biased second raw moment estimate
            var_acc[i] = beta_two * var_acc[i] + (1.0f - beta_two) * g_sq;

            // Maintain the maximum of past second moments (AmsGrad rule)
            if (var_acc[i] > var_max[i]) var_max[i] = var_acc[i];

            // Parameter update with fused denominator computation
            float denom = sqrtf(var_max[i]) + epsilon;
            theta[i] = theta[i] - rate * moment[i] / denom;

            i = i + 1;
        }
    }
};

int main() {
    // Deterministic seed for reproducibility
    srand(42);

    // Problem size (must not exceed MAX_N)
    int N = MAX_N;   // use the maximum allowed dimension for stress

    // Hyper‑parameters for AmsGrad
    float learning_rate = 0.01f;
    float beta1 = 0.9f;
    float beta2 = 0.999f;
    float eps = 1e-8f;

    // Instantiate optimizer
    AmsGrad optimizer(N, learning_rate, beta1, beta2, eps);

    // Initialise parameters with deterministic pseudo‑random values in [-1, 1]
    int idx = 0;
    while (idx < N) {
        optimizer.theta[idx] = (float)rand() / (float)RAND_MAX * 2.0f - 1.0f;
        idx = idx + 1;
    }

    // Number of optimization steps (larger for stress)
    int steps = 500;

    // Main optimization loop
    int t = 0;
    while (t < steps) {
        // Generate a fresh deterministic gradient vector
        float grad_vec[MAX_N];
        int j = 0;
        while (j < N) {
            grad_vec[j] = (float)rand() / (float)RAND_MAX * 2.0f - 1.0f;
            j = j + 1;
        }

        // Perform one AmsGrad update
        optimizer.step(grad_vec);

        t = t + 1;
    }

    // Output first ten updated parameters as a sanity check
    std::cout << "First ten parameters after " << steps << " steps:\n";
    int k = 0;
    while (k < 10 && k < N) {
        std::cout << "theta[" << k << "] = " << optimizer.theta[k] << "\n";
        k = k + 1;
    }

    return 0;
}
