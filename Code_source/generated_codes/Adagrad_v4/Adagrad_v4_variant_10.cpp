#include <iostream>
#include <cmath>

/* LLM input variant 10: large-safe-stress */

// helper: fill weight, gradient and accumulator with deterministic values
void fill_data(float* w, float* g, float* a, int n) {
    for (int i = 0; i < n; ++i) {
        // weight cycles between positive and negative base values, with a small offset
        w[i] = (i % 2 == 0 ? 0.5f : -0.3f) + 0.001f * static_cast<float>(i);
        // gradient cycles similarly, with a different base
        g[i] = (i % 3 == 0 ? 0.1f : -0.2f) + 0.0005f * static_cast<float>(i);
        a[i] = 0.0f; // accumulator starts at zero
    }
}

// helper: one Adagrad step, generalized for any dimension
void adagrad_step(float* w, float* g, float* a, float lr, float eps, int n) {
    for (int i = 0; i < n; ++i) {
        float grad = g[i];
        float sq   = grad * grad;
        a[i]       = a[i] + sq;
        float denom = std::sqrt(a[i]) + eps;
        float adj_lr = lr / denom;
        w[i]       = w[i] - adj_lr * grad;
    }
}

// helper: print current weights
void show_weights(const float* w, int n) {
    std::cout << "weights: ";
    for (int i = 0; i < n; ++i) {
        std::cout << w[i] << ' ';
    }
    std::cout << '\n';
}

int main() {
    const int dim = 1000;                // larger, yet safe size
    float* weight = new float[dim];
    float* grad   = new float[dim];
    float* accum  = new float[dim];

    fill_data(weight, grad, accum, dim);

    const float learn_rate = 0.1f;      // base learning rate
    const float epsilon    = 1e-6f;    // small constant to avoid div‑0

    std::cout << "Initial ";
    show_weights(weight, dim);

    // run a few Adagrad updates
    for (int step = 0; step < 5; ++step) {
        adagrad_step(weight, grad, accum, learn_rate, epsilon, dim);
        std::cout << "After step " << step + 1 << ' ';
        show_weights(weight, dim);
    }

    delete[] weight;
    delete[] grad;
    delete[] accum;
    return 0;
}
