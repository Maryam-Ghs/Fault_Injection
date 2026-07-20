// AmsGrad optimizer implementation – version #5
// ------------------------------------------------
// Uses only float and int, stack‑allocated arrays and a modular helper‑function design.
/* LLM input variant 10: large-safe-stress */

#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>

// ---------- helper functions ----------
void init_theta(float *theta, int sz) {
    // random initialization in range [-1, 1]
    int i = 0;
    while (i < sz) {
        theta[i] = (float(rand()) / RAND_MAX) * 2.0f - 1.0f;
        ++i;
    }
}

void zero_state(float *arr, int sz) {
    int i = 0;
    while (i < sz) {
        arr[i] = 0.0f;
        ++i;
    }
}

void compute_grad(float *grad, const float *theta, int sz) {
    // Simple quadratic loss: L = 0.5 * sum(theta_i^2)
    // Gradient is just theta itself.
    int i = 0;
    while (i < sz) {
        grad[i] = theta[i];
        ++i;
    }
}

void amsgrad_update(
    float *theta, float *first_m, float *second_v,
    float *max_v, float *grad, int sz,
    float lr, float beta1, float beta2, float eps)
{
    int i = 0;
    while (i < sz) {
        // fused updates for moments
        first_m[i] = beta1 * first_m[i] + (1.0f - beta1) * grad[i];
        second_v[i] = beta2 * second_v[i] + (1.0f - beta2) * grad[i] * grad[i];
        // keep the maximum second moment
        max_v[i] = (second_v[i] > max_v[i]) ? second_v[i] : max_v[i];
        // parameter update (fused expression)
        theta[i] -= lr * first_m[i] / (sqrtf(max_v[i]) + eps);
        ++i;
    }
}

void show_theta(const float *theta, int sz) {
    std::cout << "[ ";
    int i = 0;
    while (i < sz) {
        std::cout << theta[i];
        if (i + 1 < sz) std::cout << ", ";
        ++i;
    }
    std::cout << " ]\n";
}

// ---------- main ----------
int main() {
    std::srand(static_cast<unsigned>(std::time(0)));

    // problem size (stack allocation)
    int dim = 1000;  // increased dimension for stress testing

    float theta[1000];
    float first_m[1000];
    float second_v[1000];
    float max_v[1000];
    float grad[1000];

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

    int steps = 5000;  // increased number of optimization steps
    int step = 0;
    while (step < steps) {
        compute_grad(grad, theta, dim);
        amsgrad_update(theta, first_m, second_v, max_v, grad,
                       dim, learning_rate, b1, b2, epsilon);

        // occasional reporting
        if ((step + 1) % 1000 == 0) {
            std::cout << "After " << step + 1 << " steps: ";
            show_theta(theta, dim);
        }
        ++step;
    }

    // final result
    std::cout << "Final parameters: ";
    show_theta(theta, dim);

    return 0;
}
