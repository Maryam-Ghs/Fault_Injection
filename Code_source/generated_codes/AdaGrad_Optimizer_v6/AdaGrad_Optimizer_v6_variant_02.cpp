#include <iostream>
#include <vector>
#include <cmath>

/* LLM input variant 2: small-diverse */

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
    // deterministic test vectors with varied ordinary values
    std::vector<float> params = {4.5f, -1.2f, 0.0f, 2.8f};
    std::vector<std::vector<float>> grads = {
        {0.2f, -0.1f, 0.05f, -0.02f},
        {-0.03f, 0.04f, -0.01f, 0.015f},
        {0.005f, -0.007f, 0.003f, -0.002f}
    };

    float learning_rate = 0.3f;
    float epsilon = 1e-5f;
    AdaGrad optimizer(learning_rate, epsilon, params.size());

    std::cout << "AdaGrad optimizer version #6 demo\n";
    std::cout << "Initial parameters: ";
    for (float v : params) std::cout << v << ' ';
    std::cout << "\n\n";

    // loop-heavy iterative optimisation
    for (size_t step = 0; step < grads.size(); ++step) {
        const std::vector<float>& cur_grad = grads[step];
        optimizer.step(params, cur_grad);

        std::cout << "After step " << step + 1 << ": ";
        for (float v : params) std::cout << v << ' ';
        std::cout << "\n";
    }

    return 0;
}
