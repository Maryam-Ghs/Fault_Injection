#include <iostream>
#include <vector>
#include <cmath>

/* LLM input variant 8: sparse-skewed */

class AdaGrad {
public:
    // version #6
    float rate;
    float fudge;
    std::vector<float> accum;   // sum of squared gradients

    AdaGrad(float lr, float eps, size_t dim) {
        rate = lr;
        fudge = eps;
        accum.resize(dim);
        // initialise accumulator with zeros
        for (size_t i = 0; i < dim; ++i) {
            accum[i] = 0.0f;
        }
    }

    // Perform one optimisation step
    void step(std::vector<float>& param_vec,
              const std::vector<float>& grad_vec) {
        size_t dim = param_vec.size();

        // first loop: update accumulator
        for (size_t i = 0; i < dim; ++i) {
            float g_sq = grad_vec[i] * grad_vec[i];
            accum[i] = accum[i] + g_sq;          // accumulate squared gradient
        }

        // second loop: update parameters using accumulated info
        for (size_t i = 0; i < dim; ++i) {
            float root = std::sqrt(accum[i]);    // sqrt of accumulator
            float adj_lr = rate / (root + fudge); // adapt learning rate
            float delta = adj_lr * grad_vec[i];   // scaled gradient
            param_vec[i] = param_vec[i] - delta;  // parameter update
        }
    }
};

int main() {
    const size_t DIM = 1000;          // large, mostly empty parameter vector
    const size_t STEPS = 10;          // number of gradient steps
    // deterministic test vectors: sparse parameters
    std::vector<float> params(DIM, 0.0f);
    // initialise a few non‑zero entries to create imbalance
    params[0] = 1.0f;
    params[1] = 2.0f;
    params[2] = 3.0f;
    params[50] = 0.5f;
    params[200] = -1.5f;

    // generate sparse gradient steps
    std::vector<std::vector<float>> grads;
    grads.reserve(STEPS);
    for (size_t step = 0; step < STEPS; ++step) {
        std::vector<float> g(DIM, 0.0f);
        // introduce a few non‑zero gradients at scattered indices
        if (step % 2 == 0) {
            g[0] = 0.1f * (step + 1);
            g[50] = -0.2f * (step + 1);
        } else {
            g[2] = 0.05f * (step + 1);
            g[200] = -0.04f * (step + 1);
        }
        grads.push_back(std::move(g));
    }

    float learning_rate = 0.5f;
    float epsilon = 1e-6f;
    AdaGrad optimizer(learning_rate, epsilon, params.size());

    std::cout << "AdaGrad optimizer version #6 demo (sparse‑skewed input)\n";
    std::cout << "Initial parameters (first 10 shown): ";
    for (size_t i = 0; i < 10 && i < params.size(); ++i) std::cout << params[i] << ' ';
    std::cout << "\n\n";

    // loop-heavy iterative optimisation
    for (size_t step = 0; step < grads.size(); ++step) {
        const std::vector<float>& cur_grad = grads[step];
        optimizer.step(params, cur_grad);

        std::cout << "After step " << step + 1 << " (first 10 params): ";
        for (size_t i = 0; i < 10 && i < params.size(); ++i) std::cout << params[i] << ' ';
        std::cout << "\n";
    }

    return 0;
}
