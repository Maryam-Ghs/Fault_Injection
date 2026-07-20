// AdaGrad Optimizer – Version #10
/* LLM input variant 6: ordered-structured */
#include <iostream>
#include <cmath>

// ------------------------------------------------------------
// Helper: initialise parameters (weights) on the heap
// ------------------------------------------------------------
void init_weights(float* w, int d) {
    // structured ascending symmetric values for d = 4
    w[0] = -3.0f;
    w[1] = -1.0f;
    w[2] =  1.0f;
    w[3] =  3.0f;
}

// ------------------------------------------------------------
// Helper: initialise accumulator (sum of squares of grads)
// ------------------------------------------------------------
void init_accumulator(float* a, int d) {
    // manual unrolling for d = 4
    a[0] = 0.0f;
    a[1] = 0.0f;
    a[2] = 0.0f;
    a[3] = 0.0f;
}

// ------------------------------------------------------------
// Helper: compute a simple gradient (here 2 * w)
// ------------------------------------------------------------
void compute_gradient(float* w, float* g, int d) {
    // manual unrolling for d = 4
    g[0] = 2.0f * w[0];
    g[1] = 2.0f * w[1];
    g[2] = 2.0f * w[2];
    g[3] = 2.0f * w[3];
}

// ------------------------------------------------------------
// Helper: one AdaGrad update step (manual unrolling)
// ------------------------------------------------------------
void adagrad_update(float* w, float* g, float* a, float lr, float eps, int d) {
    // index 0
    a[0] = a[0] + g[0] * g[0];
    float inv0 = 1.0f / sqrtf(a[0] + eps);
    w[0] = w[0] - lr * g[0] * inv0;

    // index 1
    a[1] = a[1] + g[1] * g[1];
    float inv1 = 1.0f / sqrtf(a[1] + eps);
    w[1] = w[1] - lr * g[1] * inv1;

    // index 2
    a[2] = a[2] + g[2] * g[2];
    float inv2 = 1.0f / sqrtf(a[2] + eps);
    w[2] = w[2] - lr * g[2] * inv2;

    // index 3
    a[3] = a[3] + g[3] * g[3];
    float inv3 = 1.0f / sqrtf(a[3] + eps);
    w[3] = w[3] - lr * g[3] * inv3;
}

// ------------------------------------------------------------
// Main driver
// ------------------------------------------------------------
int main() {
    // dimension of the problem (fixed to 4 for this demo)
    int dim = 4;

    // allocate all arrays on the heap
    float* weight   = new float[dim];
    float* grad     = new float[dim];
    float* accum    = new float[dim];

    // hyper‑parameters
    float learn_rate = 0.1f;
    float epsilon    = 1e-8f;

    // initialise structures
    init_weights(weight, dim);
    init_accumulator(accum, dim);

    // run a few epochs
    int epoch = 0;
    while (epoch < 5) {
        // compute gradient for current weights
        compute_gradient(weight, grad, dim);

        // AdaGrad update
        adagrad_update(weight, grad, accum, learn_rate, epsilon, dim);

        // print current weights
        std::cout << "Epoch " << epoch + 1 << ": ";
        std::cout << "[" << weight[0] << ", " << weight[1] << ", "
                  << weight[2] << ", " << weight[3] << "]" << std::endl;

        epoch = epoch + 1;   // manual increment
    }

    // clean up heap memory
    delete[] weight;
    delete[] grad;
    delete[] accum;

    return 0;
}
