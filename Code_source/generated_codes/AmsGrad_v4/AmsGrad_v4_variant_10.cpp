#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>

/* LLM input variant 10: large-safe-stress */

class AmsGrad {
public:
    AmsGrad(int sz, float learning, float beta_one, float beta_two, float epsilon) {
        size = sz;
        lr = learning;
        b1 = beta_one;
        b2 = beta_two;
        eps = epsilon;

        param.assign(size, 0.0f);
        m.assign(size, 0.0f);
        v.assign(size, 0.0f);
        vmax.assign(size, 0.0f);
    }

    void init_params() {
        for (int i = 0; i < size; ++i) {
            param[i] = (static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f; // [-1,1]
        }
    }

    void step(const std::vector<float>& grad) {
        int i = 0;
        while (i < size) {
            // fused first‑moment update
            m[i] = b1 * m[i] + (1.0f - b1) * grad[i];

            // fused second‑moment update
            float g2 = grad[i] * grad[i];
            v[i] = b2 * v[i] + (1.0f - b2) * g2;

            // max of second moments
            float cand = v[i];
            vmax[i] = (vmax[i] > cand) ? vmax[i] : cand;

            // denominator (sqrt + eps) fused
            float denom = std::sqrt(vmax[i]) + eps;

            // parameter update (fused division and multiplication)
            param[i] = param[i] - lr * m[i] / denom;

            ++i;
        }
    }

    const std::vector<float>& get_params() const {
        return param;
    }

private:
    int size;
    float lr, b1, b2, eps;
    std::vector<float> param;
    std::vector<float> m;
    std::vector<float> v;
    std::vector<float> vmax;
};

int main() {
    // Fixed seed for deterministic behavior
    std::srand(0);

    // Large but safe problem size
    int dim = 20000;               // 20,000‑dimensional parameter vector
    float learn_rate = 0.01f;
    float beta_one = 0.9f;
    float beta_two = 0.999f;
    float epsilon = 1e-8f;

    AmsGrad optimizer(dim, learn_rate, beta_one, beta_two, epsilon);
    optimizer.init_params();

    int epochs = 10;               // increased number of epochs for stress testing
    int epoch = 0;
    while (epoch < epochs) {
        // generate a deterministic gradient vector
        std::vector<float> grad(dim);
        int j = 0;
        while (j < dim) {
            // Use a simple linear congruential pattern based on index for reproducibility
            grad[j] = (static_cast<float>((j * 31 + 7) % 1000) / 1000.0f) * 2.0f - 1.0f; // [-1,1]
            ++j;
        }

        optimizer.step(grad);

        const std::vector<float>& cur = optimizer.get_params();
        std::cout << "Epoch " << epoch + 1 << " parameters (first 10 shown):\n";
        // Output only first 10 values to keep output manageable
        int limit = dim < 10 ? dim : 10;
        int k = 0;
        while (k < limit) {
            std::cout << cur[k] << (k + 1 == limit ? '\n' : ' ');
            ++k;
        }

        ++epoch;
    }

    return 0;
}
