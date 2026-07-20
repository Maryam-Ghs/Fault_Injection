/* LLM input variant 10: large-safe-stress */
#include <iostream>
#include <vector>
#include <cmath>

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
    // deterministic large test vectors (size 1000)
    const size_t DIM = 1000;
    const size_t STEPS = 1000;

    std::vector<float> params(DIM);
    for (size_t i = 0; i < DIM; ++i) {
        params[i] = 1.0f + static_cast<float>(i) * 0.001f; // values 1.0, 1.001, ...
    }

    // generate deterministic gradient steps
    std::vector<std::vector<float>> grads(STEPS, std::vector<float>(DIM));
    for (size_t step = 0; step < STEPS; ++step) {
        for (size_t i = 0; i < DIM; ++i) {
            // simple pattern: small magnitude, alternating sign every other element
            float base = static_cast<float>((((step + 1) * (i + 1)) % 100)) * 0.001f;
            grads[step][i] = (i % 2 == 0) ? base : -base;
        }
    }

    float learning_rate = 0.5f;
    float epsilon = 1e-6f;
    AdaGrad optimizer(learning_rate, epsilon, params.size());

    std::cout << "AdaGrad optimizer version #6 demo (large-safe-stress)\n";
    std::cout << "Initial parameters (first 10 shown): ";
    for (size_t i = 0; i < 10; ++i) std::cout << params[i] << ' ';
    std::cout << "\n\n";

    // loop-heavy iterative optimisation
    for (size_t step = 0; step < grads.size(); ++step) {
        const std::vector<float>& cur_grad = grads[step];
        optimizer.step(params, cur_grad);

        if ((step + 1) % 100 == 0) { // occasional output to keep runtime reasonable
            std::cout << "After step " << step + 1 << " (first 10 params): ";
            for (size_t i = 0; i < 10; ++i) std::cout << params[i] << ' ';
            std::cout << "\n";
        }
    }

    std::cout << "\nFinal parameters (first 10 shown): ";
    for (size_t i = 0; i < 10; ++i) std::cout << params[i] << ' ';
    std::cout << "\n";

    return 0;
}
