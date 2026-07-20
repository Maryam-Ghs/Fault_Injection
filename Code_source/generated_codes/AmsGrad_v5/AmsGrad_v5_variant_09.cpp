/* LLM input variant 9: medium-deterministic-random */
// AmsGrad optimizer implementation – version #5
// ------------------------------------------------
// Uses only float and int, stack‑allocated arrays and a modular helper‑function design.

#include <iostream>
#include <cmath>
#include <cstdint>

// ---------- helper functions ----------
void init_theta(float *theta, int sz) {
    // deterministic pseudo‑random initialization in range [-1, 1]
    static uint32_t seed = 123456789u;
    for (int i = 0; i < sz; ++i) {
        seed = (1103515245u * seed + 12345u) & 0x7fffffffu;
        theta[i] = (static_cast<float>(seed) / 2147483647.0f) * 2.0f - 1.0f;
    }
}

void zero_state(float *arr, int sz) {
    for (int i = 0; i < sz; ++i) {
        arr[i] = 0.0f;
    }
}

void compute_grad(float *grad, const float *theta, int sz) {
    // Simple quadratic loss: L = 0.5 * sum(theta_i^2)
    // Gradient is just theta itself.
    for (int i = 0; i < sz; ++i) {
        grad[i] = theta[i];
    }
}

void amsgrad_update(
    float *theta, float *first_m, float *second_v,
    float *max_v, float *grad, int sz,
    float lr, float beta1, float beta2, float eps)
{
    for (int i = 0; i < sz; ++i) {
        // fused updates for moments
        first_m[i] = beta1 * first_m[i] + (1.0f - beta1) * grad[i];
        second_v[i] = beta2 * second_v[i] + (1.0f - beta2) * grad[i] * grad[i];
        // keep the maximum second moment
        max_v[i] = (second_v[i] > max_v[i]) ? second_v[i] : max_v[i];
        // parameter update (fused expression)
        theta[i] -= lr * first_m[i] / (sqrtf(max_v[i]) + eps);
    }
}

void show_theta(const float *theta, int sz) {
    std::cout << "[ ";
    for (int i = 0; i < sz; ++i) {
        std::cout << theta[i];
        if (i + 1 < sz) std::cout << ", ";
    }
    std::cout << " ]\n";
}

// ---------- main ----------
int main() {
    // problem size (stack allocation)
    const int dim = 7;

    float theta[dim];
    float first_m[dim];
    float second_v[dim];
    float max_v[dim];
    float grad[dim];

    // initialise everything
    init_theta(theta, dim);
    zero_state(first_m, dim);
    zero_state(second_v, dim);
    zero_state(max_v, dim);

    // hyper‑parameters (no const allowed)
    float learning_rate = 0.01f;
    float b1 = 0.9f;
    float b2 = 0.999f;
    float epsilon = 1e-8f;

    const int steps = 100;
    for (int step = 0; step < steps; ++step) {
        compute_grad(grad, theta, dim);
        amsgrad_update(theta, first_m, second_v, max_v, grad,
                       dim, learning_rate, b1, b2, epsilon);

        // occasional reporting
        if ((step + 1) % 25 == 0) {
            std::cout << "After " << step + 1 << " steps: ";
            show_theta(theta, dim);
        }
    }

    // final result
    std::cout << "Final parameters: ";
    show_theta(theta, dim);

    return 0;
}
