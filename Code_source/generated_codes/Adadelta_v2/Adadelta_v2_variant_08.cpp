#include <iostream>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <algorithm>

/* LLM input variant 8: sparse-skewed */

// ------------------------------------------------------------
// Helper: fill a vector with random floats in range [-1, 1]
// ------------------------------------------------------------
void fill_random(std::vector<float>& vec) {
    for (size_t i = 0; i < vec.size(); ++i) {
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
        float grad_sq = grad[i] * grad[i];
        avg_sq_grad[i] = decay * avg_sq_grad[i] + (1.0f - decay) * grad_sq;

        float rms_upd = std::sqrt(avg_sq_upd[i] + eps);
        float rms_grad = std::sqrt(avg_sq_grad[i] + eps);

        float upd = - (rms_upd / rms_grad) * grad[i];

        float upd_sq = upd * upd;
        avg_sq_upd[i] = decay * avg_sq_upd[i] + (1.0f - decay) * upd_sq;

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

    // problem size: medium random array
    int dim = 250;                         // moderate size
    std::vector<float> param_vec(dim);
    std::vector<float> target_vec(dim);
    std::vector<float> grad_vec(dim);
    std::vector<float> avg_sq_grad(dim);
    std::vector<float> avg_sq_upd(dim);

    // initialise vectors with random data (will be overwritten for sparsity)
    fill_random(param_vec);
    fill_random(target_vec);

    // create a sparse, clustered, skewed input: mostly zeros, few non‑zero blocks
    std::fill(param_vec.begin(), param_vec.end(), 0.0f);
    std::fill(target_vec.begin(), target_vec.end(), 0.0f);

    // first cluster at beginning
    param_vec[0] = 0.92f;  target_vec[0] = 0.45f;
    param_vec[1] = -0.81f; target_vec[1] = -0.33f;
    param_vec[2] = 0.73f;  target_vec[2] = 0.20f;
    param_vec[3] = -0.60f; target_vec[3] = -0.10f;
    param_vec[4] = 0.55f;  target_vec[4] = 0.05f;

    // second cluster in the middle
    param_vec[120] = -0.77f; target_vec[120] = -0.40f;
    param_vec[121] = 0.68f;  target_vec[121] = 0.30f;
    param_vec[122] = -0.52f; target_vec[122] = -0.25f;
    param_vec[123] = 0.49f;  target_vec[123] = 0.15f;
    param_vec[124] = -0.41f; target_vec[124] = -0.12f;

    // third small cluster near the end
    param_vec[240] = 0.88f; target_vec[240] = 0.60f;
    param_vec[241] = -0.70f; target_vec[241] = -0.45f;
    param_vec[242] = 0.33f; target_vec[242] = 0.10f;

    // zero the accumulators
    for (int i = 0; i < dim; ++i) {
        avg_sq_grad[i] = 0.0f;
        avg_sq_upd[i] = 0.0f;
    }

    // hyper‑parameters (float only)
    float decay = 0.95f;   // rho
    float eps = 1e-6f;     // epsilon
    int epochs = 15;

    // training loop
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
