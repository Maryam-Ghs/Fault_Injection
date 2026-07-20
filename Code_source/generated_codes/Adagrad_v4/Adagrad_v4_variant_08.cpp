/* LLM input variant 8: sparse-skewed */
#include <iostream>
#include <cmath>

// helper: fill weight, gradient and accumulator with sparse preset numbers
void fill_data(float* w, float* g, float* a, int n) {
    // initialize everything to zero (sparse baseline)
    for (int i = 0; i < n; ++i) {
        w[i] = 0.0f;
        g[i] = 0.0f;
        a[i] = 0.0f;
    }
    // introduce a few non‑zero, clustered entries to create skew
    if (n > 0) { w[0] = 0.5f;   g[0] = 0.1f; }
    if (n > 10) { w[10] = -0.3f; g[10] = -0.2f; }
    if (n > 20) { w[20] = 0.8f;  g[20] = 0.05f; }
    if (n > 30) { w[30] = -0.1f; g[30] = 0.3f; }
    if (n > 40) { w[40] = 0.6f;  g[40] = -0.15f; }
}

// helper: one Adagrad step, generalized for any dimension
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
    int dim = 50;                    // larger size with many zeros
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
