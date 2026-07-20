#include <iostream>
#include <vector>
#include <cmath>

/* LLM input variant 9: medium-deterministic-random */

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
    // deterministic test vectors (medium-sized, pseudo‑random looking)
    std::vector<float> params = {0.5f, 1.5f, 2.5f, 3.5f, 4.5f, 5.5f, 6.5f, 7.5f, 8.5f, 9.5f};
    std::vector<std::vector<float>> grads = {
        { 0.067f, -0.023f,  0.045f, -0.089f,  0.012f, -0.034f,  0.058f, -0.071f,  0.019f, -0.005f},
        {-0.054f,  0.031f, -0.060f,  0.042f, -0.017f,  0.080f, -0.022f,  0.037f, -0.048f,  0.011f},
        { 0.039f, -0.015f,  0.027f, -0.040f,  0.009f, -0.025f,  0.033f, -0.048f,  0.014f, -0.002f},
        {-0.041f,  0.018f, -0.050f,  0.036f, -0.012f,  0.069f, -0.019f,  0.030f, -0.041f,  0.009f},
        { 0.028f, -0.010f,  0.019f, -0.027f,  0.006f, -0.017f,  0.022f, -0.032f,  0.009f, -0.001f},
        {-0.022f,  0.008f, -0.014f,  0.020f, -0.004f,  0.050f, -0.012f,  0.018f, -0.025f,  0.005f},
        { 0.015f, -0.006f,  0.010f, -0.014f,  0.003f, -0.010f,  0.012f, -0.018f,  0.006f, -0.0005f},
        {-0.011f,  0.004f, -0.008f,  0.012f, -0.002f,  0.035f, -0.008f,  0.012f, -0.017f,  0.003f},
        { 0.008f, -0.003f,  0.006f, -0.009f,  0.0015f, -0.007f,  0.009f, -0.013f,  0.004f, -0.0002f},
        {-0.006f,  0.0025f, -0.004f,  0.008f, -0.001f,  0.025f, -0.006f,  0.009f, -0.012f,  0.002f},
        { 0.004f, -0.0015f,  0.003f, -0.006f,  0.0008f, -0.005f,  0.006f, -0.008f,  0.003f, -0.0001f},
        {-0.003f,  0.0012f, -0.0025f,  0.005f, -0.0007f,  0.018f, -0.004f,  0.006f, -0.009f,  0.0015f}
    };

    float learning_rate = 0.3f;
    float epsilon = 1e-8f;
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
