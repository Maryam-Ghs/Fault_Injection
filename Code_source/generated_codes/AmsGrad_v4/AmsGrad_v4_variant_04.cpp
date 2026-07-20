#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>

/* LLM input variant 4: signed-extremes */

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
            // Mix negative, zero, and positive values deterministically
            if (i % 3 == 0) param[i] = -1.0f;
            else if (i % 3 == 1) param[i] = 0.0f;
            else param[i] = 1.0f;
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
    // Deterministic input; no need for random seed
    int dim = 10;                     // medium sized array
    float learn_rate = 0.01f;
    float beta_one = 0.9f;
    float beta_two = 0.999f;
    float epsilon = 1e-8f;

    AmsGrad optimizer(dim, learn_rate, beta_one, beta_two, epsilon);
    optimizer.init_params();

    int epochs = 5;
    int epoch = 0;
    while (epoch < epochs) {
        // generate a deterministic gradient vector mixing negative, zero, positive
        std::vector<float> grad(dim);
        int j = 0;
        while (j < dim) {
            if ((j + epoch) % 3 == 0) grad[j] = -1.0f;
            else if ((j + epoch) % 3 == 1) grad[j] = 0.0f;
            else grad[j] = 1.0f;
            ++j;
        }

        optimizer.step(grad);

        const std::vector<float>& cur = optimizer.get_params();
        std::cout << "Epoch " << epoch + 1 << " parameters:\n";
        int k = 0;
        while (k < dim) {
            std::cout << cur[k] << (k + 1 == dim ? '\n' : ' ');
            ++k;
        }

        ++epoch;
    }

    return 0;
}
