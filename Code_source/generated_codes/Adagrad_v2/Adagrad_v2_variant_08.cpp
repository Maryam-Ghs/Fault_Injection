#include <iostream>
#include <cmath>

/* LLM input variant 8: sparse-skewed */

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
    // deterministic sparse-skewed test data: linear regression with 2 features
    const int N = 15;          // number of samples (mostly zero features)
    const int D = 2;           // feature dimension
    float* X = new float[N * D];
    float* Y = new float[N];

    // initialize all entries to zero (sparse baseline)
    for (int i = 0; i < N * D; ++i) {
        X[i] = 0.0f;
    }
    for (int i = 0; i < N; ++i) {
        Y[i] = 0.0f;
    }

    // populate a few clustered, non‑zero samples
    // Sample 0: x = [10, 0], y = 15
    X[0 * D + 0] = 10.0f; X[0 * D + 1] = 0.0f;
    Y[0] = 15.0f;
    // Sample 1: x = [0, 5], y = 10
    X[1 * D + 0] = 0.0f; X[1 * D + 1] = 5.0f;
    Y[1] = 10.0f;
    // Sample 2: x = [8, 2], y = 16
    X[2 * D + 0] = 8.0f; X[2 * D + 1] = 2.0f;
    Y[2] = 16.0f;
    // Sample 3: x = [9, 1], y = 15.5
    X[3 * D + 0] = 9.0f; X[3 * D + 1] = 1.0f;
    Y[3] = 15.5f;
    // Sample 4: x = [7, 3], y = 15.0
    X[4 * D + 0] = 7.0f; X[4 * D + 1] = 3.0f;
    Y[4] = 15.0f;
    // Remaining samples (5‑14) stay zero, Y stays zero (highly sparse region)

    // hyper‑parameters
    float learning_rate = 1.0f;
    float eps = 1e-8f;
    int epochs = 200;

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
