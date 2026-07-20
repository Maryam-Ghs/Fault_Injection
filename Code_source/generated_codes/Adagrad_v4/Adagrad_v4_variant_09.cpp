#include <iostream>
#include <cmath>

/* LLM input variant 9: medium-deterministic-random */

// helper: fill weight, gradient and accumulator with deterministic pseudo‑random numbers
void fill_data(float* w, float* g, float* a, int n) {
    unsigned int seed = 0x9e3779b9u; // fixed seed
    for (int i = 0; i < n; ++i) {
        // generate weight in [-1.0, 1.0)
        seed = seed * 1664525u + 1013904223u;
        w[i] = ((seed >> 16) & 0xFFFF) / 32768.0f - 1.0f;

        // generate gradient in [-0.5, 0.5)
        seed = seed * 1664525u + 1013904223u;
        g[i] = ((seed >> 16) & 0xFFFF) / 65536.0f - 0.5f;

        a[i] = 0.0f; // accumulator starts at zero
    }
}

// helper: one Adagrad step, manually unrolled for 4 elements
void adagrad_step(float* w, float* g, float* a, float lr, float eps, int n) {
    // element 0
    float grad0 = g[0];
    float sq0   = grad0 * grad0;
    a[0]        = a[0] + sq0;
    float denom0 = std::sqrt(a[0]) + eps;
    float adj_lr0 = lr / denom0;
    w[0]        = w[0] - adj_lr0 * grad0;

    // element 1
    float grad1 = g[1];
    float sq1   = grad1 * grad1;
    a[1]        = a[1] + sq1;
    float denom1 = std::sqrt(a[1]) + eps;
    float adj_lr1 = lr / denom1;
    w[1]        = w[1] - adj_lr1 * grad1;

    // element 2
    float grad2 = g[2];
    float sq2   = grad2 * grad2;
    a[2]        = a[2] + sq2;
    float denom2 = std::sqrt(a[2]) + eps;
    float adj_lr2 = lr / denom2;
    w[2]        = w[2] - adj_lr2 * grad2;

    // element 3
    float grad3 = g[3];
    float sq3   = grad3 * grad3;
    a[3]        = a[3] + sq3;
    float denom3 = std::sqrt(a[3]) + eps;
    float adj_lr3 = lr / denom3;
    w[3]        = w[3] - adj_lr3 * grad3;
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
    int dim = 4;                     // small fixed size
    float* weight   = new float[dim];
    float* grad     = new float[dim];
    float* accum    = new float[dim];

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
