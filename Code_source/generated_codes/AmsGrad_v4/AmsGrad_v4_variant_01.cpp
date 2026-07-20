#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>

/* LLM input variant 1: minimal-boundary */

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
            param[i] = 0.5f; // deterministic non‑trivial init
        }
    }

    void step(const std::vector<float>& grad) {
        int i = 0;
        while (i < size) {
            m[i] = b1 * m[i] + (1.0f - b1) * grad[i];
            float g2 = grad[i] * grad[i];
            v[i] = b2 * v[i] + (1.0f - b2) * g2;
            float cand = v[i];
            vmax[i] = (vmax[i] > cand) ? vmax[i] : cand;
            float denom = std::sqrt(vmax[i]) + eps;
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
    std::srand(0); // deterministic seed

    int dim = 1;                     // minimal non‑trivial dimension
    float learn_rate = 1e-6f;       // smallest positive learning rate
    float beta_one = 0.0f;           // boundary value
    float beta_two = 0.0f;           // boundary value
    float epsilon = 1e-8f;          // minimal positive epsilon

    AmsGrad optimizer(dim, learn_rate, beta_one, beta_two, epsilon);
    optimizer.init_params();

    int epochs = 1;                  // minimal non‑trivial epoch count
    int epoch = 0;
    while (epoch < epochs) {
        std::vector<float> grad(dim);
        for (int j = 0; j < dim; ++j) {
            grad[j] = -0.5f; // deterministic gradient
        }

        optimizer.step(grad);

        const std::vector<float>& cur = optimizer.get_params();
        std::cout << "Epoch " << epoch + 1 << " parameters:\n";
        for (int k = 0; k < dim; ++k) {
            std::cout << cur[k] << (k + 1 == dim ? '\n' : ' ');
        }

        ++epoch;
    }

    return 0;
}
