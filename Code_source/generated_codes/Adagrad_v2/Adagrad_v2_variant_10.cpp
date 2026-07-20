#include <iostream>
#include <cmath>

/* LLM input variant 10: large-safe-stress */

class AdaGradOptimizer {
public:
    AdaGradOptimizer(int param_dim, float base_lr, float eps) {
        dim = param_dim;
        lr0 = base_lr;
        epsilon = eps;
        // allocate accumulator for squared gradients
        accum = new float[dim];
        // allocate parameter vector
        param = new float[dim];
        for (int i = 0; i < dim; ++i) {
            accum[i] = 0.0f;
            param[i] = 0.0f;               // start from zero
        }
    }

    ~AdaGradOptimizer() {
        delete[] accum;
        delete[] param;
    }

    // one step of Adagrad for a given gradient vector
    void step(const float* grad) {
        for (int i = 0; i < dim; ++i) {
            // update accumulated squared gradient
            accum[i] = accum[i] + grad[i] * grad[i];
        }
        for (int i = 0; i < dim; ++i) {
            // compute adjusted learning rate for each dimension
            float adj_lr = lr0 / (std::sqrt(accum[i]) + epsilon);
            // update parameter
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
    // deterministic large‑scale test data: synthetic linear regression
    const int N = 2000;          // number of samples (large but safe)
    const int D = 100;          // feature dimension (large but safe)
    float* X = new float[N * D];
    float* Y = new float[N];

    // fill X with a simple pattern: X[i][j] = (i+1) * (j+1) * 0.01f
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < D; ++j) {
            X[i * D + j] = static_cast<float>((i + 1) * (j + 1)) * 0.01f;
        }
    }

    // fill Y as the sum of features for each sample (ensures a linear relationship)
    for (int i = 0; i < N; ++i) {
        float sum = 0.0f;
        for (int j = 0; j < D; ++j) {
            sum += X[i * D + j];
        }
        Y[i] = sum;  // target = linear combination with all weights = 1
    }

    // hyper‑parameters
    float learning_rate = 0.5f;
    float eps = 1e-8f;
    int epochs = 50;

    // create optimizer
    AdaGradOptimizer opt(D, learning_rate, eps);

    // training loop (loop‑heavy, iterative)
    for (int ep = 0; ep < epochs; ++ep) {
        for (int s = 0; s < N; ++s) {
            // compute prediction: w^T x
            const float* w = opt.get_params();
            float pred = 0.0f;
            for (int j = 0; j < D; ++j) {
                pred = pred + w[j] * X[s * D + j];
            }

            // compute gradient of squared loss: 2 * (pred - y) * x
            float factor = 2.0f * (pred - Y[s]);
            float* grad = new float[D];
            for (int j = 0; j < D; ++j) {
                grad[j] = factor * X[s * D + j];
            }

            // Adagrad step
            opt.step(grad);

            delete[] grad;
        }
    }

    // output final learned parameters
    const float* final_w = opt.get_params();
    std::cout << "Learned weights after " << epochs << " epochs:\n";
    for (int i = 0; i < D; ++i) {
        std::cout << "w[" << i << "] = " << final_w[i] << "\n";
    }

    delete[] X;
    delete[] Y;
    return 0;
}
