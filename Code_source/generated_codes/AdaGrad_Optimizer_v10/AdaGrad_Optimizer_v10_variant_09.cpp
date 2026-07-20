/* LLM input variant 9: medium-deterministic-random */
// AdaGrad Optimizer – Version #10
#include <iostream>
#include <cmath>

// ------------------------------------------------------------
// Helper: initialise parameters (weights) on the heap
// ------------------------------------------------------------
void init_weights(float* w, int d) {
    // deterministic pseudo‑random‑looking values
    for (int i = 0; i < d; ++i) {
        // simple linear congruential pattern mapped to range [-8, 8]
        int val = (i * 13 + 7) % 17;          // values 0..16
        w[i] = static_cast<float>(val - 8); // shift to -8..8
    }
}

// ------------------------------------------------------------
// Helper: initialise accumulator (sum of squares of grads)
// ------------------------------------------------------------
void init_accumulator(float* a, int d) {
    for (int i = 0; i < d; ++i) {
        a[i] = 0.0f;
    }
}

// ------------------------------------------------------------
// Helper: compute a simple gradient (here 2 * w)
// ------------------------------------------------------------
void compute_gradient(float* w, float* g, int d) {
    for (int i = 0; i < d; ++i) {
        g[i] = 2.0f * w[i];
    }
}

// ------------------------------------------------------------
// Helper: one AdaGrad update step (loop based)
// ------------------------------------------------------------
void adagrad_update(float* w, float* g, float* a, float lr, float eps, int d) {
    for (int i = 0; i < d; ++i) {
        a[i] = a[i] + g[i] * g[i];
        float inv = 1.0f / sqrtf(a[i] + eps);
        w[i] = w[i] - lr * g[i] * inv;
    }
}

// ------------------------------------------------------------
// Main driver
// ------------------------------------------------------------
int main() {
    // dimension of the problem (medium size)
    const int dim = 10;

    // allocate all arrays on the heap
    float* weight = new float[dim];
    float* grad   = new float[dim];
    float* accum  = new float[dim];

    // hyper‑parameters
    const float learn_rate = 0.1f;
    const float epsilon    = 1e-8f;

    // initialise structures
    init_weights(weight, dim);
    init_accumulator(accum, dim);

    // run a few epochs
    for (int epoch = 0; epoch < 5; ++epoch) {
        // compute gradient for current weights
        compute_gradient(weight, grad, dim);

        // AdaGrad update
        adagrad_update(weight, grad, accum, learn_rate, epsilon, dim);

        // print current weights
        std::cout << "Epoch " << epoch + 1 << ": [";
        for (int i = 0; i < dim; ++i) {
            std::cout << weight[i];
            if (i + 1 < dim) std::cout << ", ";
        }
        std::cout << "]" << std::endl;
    }

    // clean up heap memory
    delete[] weight;
    delete[] grad;
    delete[] accum;

    return 0;
}
