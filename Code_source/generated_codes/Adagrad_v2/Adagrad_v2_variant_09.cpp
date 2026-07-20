#include <iostream>
#include <cmath>

/* LLM input variant 9: medium-deterministic-random */

class AdaGradOptimizer {
public:
    AdaGradOptimizer(int param_dim, float base_lr, float eps) {
        dim = param_dim;
        lr0 = base_lr;
        epsilon = eps;
        accum = new float[dim];
        param = new float[dim];
        for (int i = 0; i < dim; ++i) {
            accum[i] = 0.0f;
            param[i] = 0.0f;
        }
    }

    ~AdaGradOptimizer() {
        delete[] accum;
        delete[] param;
    }

    void step(const float* grad) {
        for (int i = 0; i < dim; ++i) {
            accum[i] = accum[i] + grad[i] * grad[i];
        }
        for (int i = 0; i < dim; ++i) {
            float adj_lr = lr0 / (std::sqrt(accum[i]) + epsilon);
            param[i] = param[i] - adj_lr * grad[i];
        }
    }

    const float* get_params() const {
        return param;
    }

private:
    int dim;
    float lr0;
    float epsilon;
    float* accum;
    float* param;
};

int main() {
    // deterministic pseudo‑random‑looking test data
    const int N = 8;          // number of samples
    const int D = 3;          // feature dimension
    float* X = new float[N * D];
    float* Y = new float[N];

    // simple deterministic generator (linear congruential style)
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < D; ++j) {
            int val = (i * 31 + j * 17) % 11;          // values in [0,10]
            X[i * D + j] = static_cast<float>(val + 1); // shift to [1,11]
        }
        // target: linear combination with fixed coefficients plus offset
        Y[i] = 2.0f * X[i * D + 0] - 0.5f * X[i * D + 1] + 1.5f * X[i * D + 2] + 3.0f;
    }

    // hyper‑parameters
    float learning_rate = 1.0f;
    float eps = 1e-8f;
    int epochs = 150;

    AdaGradOptimizer opt(D, learning_rate, eps);

    for (int ep = 0; ep < epochs; ++ep) {
        for (int s = 0; s < N; ++s) {
            const float* w = opt.get_params();
            float pred = 0.0f;
            for (int j = 0; j < D; ++j) {
                pred = pred + w[j] * X[s * D + j];
            }

            float factor = 2.0f * (pred - Y[s]);
            float* grad = new float[D];
            for (int j = 0; j < D; ++j) {
                grad[j] = factor * X[s * D + j];
            }

            opt.step(grad);
            delete[] grad;
        }
    }

    const float* final_w = opt.get_params();
    std::cout << "Learned weights after " << epochs << " epochs:\n";
    for (int i = 0; i < D; ++i) {
        std::cout << "w[" << i << "] = " << final_w[i] << "\n";
    }

    delete[] X;
    delete[] Y;
    return 0;
}
