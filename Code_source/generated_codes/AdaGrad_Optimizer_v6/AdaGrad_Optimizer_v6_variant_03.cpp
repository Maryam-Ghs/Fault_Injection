/* LLM input variant 3: zeros-and-ones */
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
    // deterministic test vectors emphasizing zeros and ones
    std::vector<float> params = {0.0f, 1.0f, 0.0f, 1.0f};
    std::vector<std::vector<float>> grads = {
        {0.0f, 0.0f, 0.0f, 0.0f},
        {1.0f, 1.0f, 1.0f, 1.0f},
        {0.0f, 1.0f, 0.0f, 1.0f},
        {1.0f, 0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 1.0f}
    };

    float learning_rate = 1.0f; // identity learning rate
    float epsilon = 1e-6f;
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
