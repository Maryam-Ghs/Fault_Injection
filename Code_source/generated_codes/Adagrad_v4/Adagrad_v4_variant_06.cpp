/* LLM input variant 6: ordered-structured */
#include <iostream>
#include <cmath>

// helper: fill weight, gradient and accumulator with ordered structured numbers
void fill_data(float* w, float* g, float* a, int n) {
    // ordered weights (ascending)
    float ordered_weights[8] = { -0.8f, -0.5f, -0.2f, 0.0f, 0.1f, 0.3f, 0.5f, 0.9f };
    // ordered gradients (ascending)
    float ordered_grads[8]   = { -0.3f, -0.2f, -0.1f, 0.0f, 0.1f, 0.2f, 0.3f, 0.4f };
    for (int i = 0; i < n; ++i) {
        w[i] = ordered_weights[i];
        g[i] = ordered_grads[i];
        a[i] = 0.0f;
    }
}

// helper: one Adagrad step, generic for any dimension
void adagrad_step(float* w, float* g, float* a, float lr, float eps, int n) {
    for (int i = 0; i < n; ++i) {
        float grad = g[i];
        float sq   = grad * grad;
        a[i]      += sq;
        float denom = std::sqrt(a[i]) + eps;
        float adj_lr = lr / denom;
        w[i]      -= adj_lr * grad;
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
    int dim = 8;                     // expanded ordered size
    float* weight = new float[dim];
    float* grad   = new float[dim];
    float* accum  = new float[dim];

    fill_data(weight, grad, accum, dim);

    float learn_rate = 0.1f;         // base learning rate
    float epsilon    = 1e-6f;        // small constant to avoid div‑0

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
