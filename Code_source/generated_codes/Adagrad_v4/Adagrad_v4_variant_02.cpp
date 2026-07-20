/* LLM input variant 2: small-diverse */
#include <iostream>
#include <cmath>

// helper: fill weight, gradient and accumulator with small preset numbers
void fill_data(float* w, float* g, float* a, int n) {
    // varied preset values
    w[0] = 1.2f;   g[0] = 0.0f;    a[0] = 0.0f;
    w[1] = -0.7f;  g[1] = 0.25f;   a[1] = 0.0f;
    w[2] = 0.0f;   g[2] = -0.15f;  a[2] = 0.0f;
    w[3] = 2.5f;   g[3] = 0.5f;    a[3] = 0.0f;
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

    float learn_rate = 0.05f;         // adjusted learning rate
    float epsilon    = 1e-5f;        // slightly larger epsilon

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
