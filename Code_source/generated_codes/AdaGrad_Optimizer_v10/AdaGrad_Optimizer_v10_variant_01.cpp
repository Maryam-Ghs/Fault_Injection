/* LLM input variant 1: minimal-boundary */
// AdaGrad Optimizer – Version #10
#include <iostream>
#include <cmath>

// ------------------------------------------------------------
// Helper: initialise parameters (weights) on the heap
// ------------------------------------------------------------
void init_weights(float* w, int d) {
    // manual unrolling for d = 1
    w[0] = 5.0f;
}

// ------------------------------------------------------------
// Helper: initialise accumulator (sum of squares of grads)
// ------------------------------------------------------------
void init_accumulator(float* a, int d) {
    // manual unrolling for d = 1
    a[0] = 0.0f;
}

// ------------------------------------------------------------
// Helper: compute a simple gradient (here 2 * w)
// ------------------------------------------------------------
void compute_gradient(float* w, float* g, int d) {
    // manual unrolling for d = 1
    g[0] = 2.0f * w[0];
}

// ------------------------------------------------------------
// Helper: one AdaGrad update step (manual unrolling)
// ------------------------------------------------------------
void adagrad_update(float* w, float* g, float* a, float lr, float eps, int d) {
    // index 0
    a[0] = a[0] + g[0] * g[0];
    float inv0 = 1.0f / sqrtf(a[0] + eps);
    w[0] = w[0] - lr * g[0] * inv0;
}

// ------------------------------------------------------------
// Main driver
// ------------------------------------------------------------
int main() {
    // dimension of the problem (minimal non‑trivial size)
    int dim = 1;

    // allocate all arrays on the heap
    float* weight = new float[dim];
    float* grad   = new float[dim];
    float* accum  = new float[dim];

    // hyper‑parameters
    float learn_rate = 0.1f;
    float epsilon    = 1e-8f;

    // initialise structures
    init_weights(weight, dim);
    init_accumulator(accum, dim);

    // run a single epoch (minimal non‑trivial count)
    int epoch = 0;
    while (epoch < 1) {
        // compute gradient for current weights
        compute_gradient(weight, grad, dim);

        // AdaGrad update
        adagrad_update(weight, grad, accum, learn_rate, epsilon, dim);

        // print current weight
        std::cout << "Epoch " << epoch + 1 << ": ";
        std::cout << "[" << weight[0] << "]" << std::endl;

        epoch = epoch + 1;   // manual increment
    }

    // clean up heap memory
    delete[] weight;
    delete[] grad;
    delete[] accum;

    return 0;
}
