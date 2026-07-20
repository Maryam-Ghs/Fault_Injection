/* LLM input variant 6: ordered-structured */
#include <iostream>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <ctime>

// ------------------------------------------------------------
// Helper: fill a vector with ordered floats in range [-1, 1]
// ------------------------------------------------------------
void fill_random(std::vector<float>& vec) {
    size_t n = vec.size();
    if (n == 0) return;
    for (size_t i = 0; i < n; ++i) {
        // linearly spaced values from -1.0 to 1.0 (inclusive)
        vec[i] = -1.0f + 2.0f * static_cast<float>(i) / static_cast<float>(n - 1);
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

    // problem size: medium ordered array
    int dim = 250;                         // moderate size
    std::vector<float> param_vec(dim);
    std::vector<float> target_vec(dim);
    std::vector<float> grad_vec(dim);
    std::vector<float> avg_sq_grad(dim);
    std::vector<float> avg_sq_upd(dim);

    // initialise vectors with ordered data
    fill_random(param_vec);
    fill_random(target_vec);
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
