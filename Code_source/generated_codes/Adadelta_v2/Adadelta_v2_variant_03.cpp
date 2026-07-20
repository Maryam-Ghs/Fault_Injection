#include <iostream>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <ctime>

/* LLM input variant 3: zeros-and-ones */

// ------------------------------------------------------------
// Helper: fill a vector with random floats in range [-1, 1]
// ------------------------------------------------------------
void fill_random(std::vector<float>& vec) {
    for (size_t i = 0; i < vec.size(); ++i) {
        // rand() returns int in [0, RAND_MAX]
        // Scale to [-1, 1] using float arithmetic only
        float r = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
        vec[i] = 2.0f * r - 1.0f;
    }
}

// ------------------------------------------------------------
// Helper: simple quadratic loss gradient  g = 2 * (w - target)
// ------------------------------------------------------------
void compute_gradient(const std::vector<float>& param,
                      const std::vector<float>& target,
                      std::vector<float>& grad) {
    for (size_t i = 0; i < param.size(); ++i) {
        // reorder: multiply first, then subtract
        grad[i] = 2.0f * param[i];
        grad[i] = grad[i] - 2.0f * target[i];
    }
}

// ------------------------------------------------------------
// Helper: one Adadelta step
// ------------------------------------------------------------
void adadelta_update(std::vector<float>& param,
                     const std::vector<float>& grad,
                     std::vector<float>& avg_sq_grad,
                     std::vector<float>& avg_sq_upd,
                     float decay,
                     float eps) {
    for (size_t i = 0; i < param.size(); ++i) {
        // update running average of squared gradients
        float grad_sq = grad[i] * grad[i];
        avg_sq_grad[i] = decay * avg_sq_grad[i] + (1.0f - decay) * grad_sq;

        // compute RMS values (reordered arithmetic)
        float rms_upd = std::sqrt(avg_sq_upd[i] + eps);
        float rms_grad = std::sqrt(avg_sq_grad[i] + eps);

        // compute parameter update
        float upd = - (rms_upd / rms_grad) * grad[i];

        // update running average of squared updates
        float upd_sq = upd * upd;
        avg_sq_upd[i] = decay * avg_sq_upd[i] + (1.0f - decay) * upd_sq;

        // apply update
        param[i] = param[i] + upd;
    }
}

// ------------------------------------------------------------
// Helper: print first N elements of a vector
// ------------------------------------------------------------
void print_some(const std::vector<float>& vec, int count) {
    int limit = count < static_cast<int>(vec.size()) ? count : static_cast<int>(vec.size());
    for (int i = 0; i < limit; ++i) {
        std::cout << vec[i] << ' ';
    }
    std::cout << std::endl;
}

// ------------------------------------------------------------
// Main driver
// ------------------------------------------------------------
int main() {
    std::srand(static_cast<unsigned>(std::time(0)));

    // problem size: emphasize zeros and ones
    int dim = 256;                         // power‑of‑two size
    std::vector<float> param_vec(dim);
    std::vector<float> target_vec(dim);
    std::vector<float> grad_vec(dim);
    std::vector<float> avg_sq_grad(dim);
    std::vector<float> avg_sq_upd(dim);

    // initialise vectors with alternating zeros and ones
    for (int i = 0; i < dim; ++i) {
        param_vec[i] = (i % 2 == 0) ? 0.0f : 1.0f;
        target_vec[i] = (i % 2 == 0) ? 1.0f : 0.0f;
    }

    // zero the accumulators
    for (int i = 0; i < dim; ++i) {
        avg_sq_grad[i] = 0.0f;
        avg_sq_upd[i] = 0.0f;
    }

    // hyper‑parameters (float only)
    float decay = 0.95f;   // rho
    float eps = 1e-6f;     // epsilon
    int epochs = 15;

    // training loop (while‑style with explicit counter)
    int it = 0;
    while (it < epochs) {
        compute_gradient(param_vec, target_vec, grad_vec);
        adadelta_update(param_vec, grad_vec, avg_sq_grad, avg_sq_upd, decay, eps);
        ++it;
    }

    // output a few results
    std::cout << "First 10 parameters after Adadelta:\n";
    print_some(param_vec, 10);
    std::cout << "First 10 target values (reference):\n";
    print_some(target_vec, 10);

    return 0;
}
