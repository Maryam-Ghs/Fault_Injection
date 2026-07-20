/* LLM input variant 9: medium-deterministic-random */
// AmsGrad optimizer – version #3
// -------------------------------------------------
// Implements AmsGrad with manual loop unrolling, reordered arithmetic,
// float‑only arithmetic, and std::vector storage.

#include <iostream>
#include <vector>
#include <cstdlib>
#include <cmath>

class AmsGrad {
public:
    // hyper‑parameters (all float as required)
    float lr;          // learning rate
    float b1;          // beta1
    float b2;          // beta2
    float eps;         // epsilon to avoid division by zero

    // state vectors
    std::vector<float> m;      // first moment
    std::vector<float> v;      // second moment
    std::vector<float> v_max;  // max of v (v̂)

    // constructor – allocate state vectors of given size
    AmsGrad(int dim, float learning_rate, float beta1, float beta2, float epsilon) {
        lr   = learning_rate;
        b1   = beta1;
        b2   = beta2;
        eps  = epsilon;

        m.assign(dim, 0.0f);
        v.assign(dim, 0.0f);
        v_max.assign(dim, 0.0f);
    }

    // single optimisation step
    void step(std::vector<float>& param, const std::vector<float>& grad) {
        int n = (int)param.size();

        // pre‑compute (1‑beta) terms, reordered for readability
        float one_minus_b1 = 1.0f - b1;
        float one_minus_b2 = 1.0f - b2;

        // manual loop unrolling – process 4 elements per iteration
        int i = 0;
        for (; i + 3 < n; i += 4) {
            // ---- element i ----
            float g0 = grad[i];
            m[i] = b1 * m[i] + one_minus_b1 * g0;
            v[i] = b2 * v[i] + one_minus_b2 * g0 * g0;
            if (v[i] > v_max[i]) v_max[i] = v[i];
            float denom0 = std::sqrt((float)v_max[i]) + eps;
            param[i] = param[i] - lr * (m[i] / denom0);

            // ---- element i+1 ----
            float g1 = grad[i + 1];
            m[i + 1] = b1 * m[i + 1] + one_minus_b1 * g1;
            v[i + 1] = b2 * v[i + 1] + one_minus_b2 * g1 * g1;
            if (v[i + 1] > v_max[i + 1]) v_max[i + 1] = v[i + 1];
            float denom1 = std::sqrt((float)v_max[i + 1]) + eps;
            param[i + 1] = param[i + 1] - lr * (m[i + 1] / denom1);

            // ---- element i+2 ----
            float g2 = grad[i + 2];
            m[i + 2] = b1 * m[i + 2] + one_minus_b1 * g2;
            v[i + 2] = b2 * v[i + 2] + one_minus_b2 * g2 * g2;
            if (v[i + 2] > v_max[i + 2]) v_max[i + 2] = v[i + 2];
            float denom2 = std::sqrt((float)v_max[i + 2]) + eps;
            param[i + 2] = param[i + 2] - lr * (m[i + 2] / denom2);

            // ---- element i+3 ----
            float g3 = grad[i + 3];
            m[i + 3] = b1 * m[i + 3] + one_minus_b1 * g3;
            v[i + 3] = b2 * v[i + 3] + one_minus_b2 * g3 * g3;
            if (v[i + 3] > v_max[i + 3]) v_max[i + 3] = v[i + 3];
            float denom3 = std::sqrt((float)v_max[i + 3]) + eps;
            param[i + 3] = param[i + 3] - lr * (m[i + 3] / denom3);
        }

        // tail loop for remaining elements (0‑3 of them)
        for (; i < n; ++i) {
            float g = grad[i];
            m[i] = b1 * m[i] + one_minus_b1 * g;
            v[i] = b2 * v[i] + one_minus_b2 * g * g;
            if (v[i] > v_max[i]) v_max[i] = v[i];
            float denom = std::sqrt((float)v_max[i]) + eps;
            param[i] = param[i] - lr * (m[i] / denom);
        }
    }
};

int main() {
    // -------------------------------------------------
    // generate a medium‑sized deterministic pseudo‑random problem
    // -------------------------------------------------
    const unsigned FIXED_SEED = 123456789u;
    std::srand(FIXED_SEED);

    const int DIM = 20;                     // medium size (changed from 16)
    std::vector<float> weights(DIM);
    std::vector<float> grads(DIM);

    // initialise weights and gradients with deterministic pseudo‑random floats in [-1, 1]
    for (int i = 0; i < DIM; ++i) {
        // simple LCG‑style deterministic pattern
        int w_raw = (i * 31 + 7) % 10000;        // 0 .. 9999
        int g_raw = (i * 47 + 13) % 10000;
        weights[i] = static_cast<float>(w_raw) / 5000.0f - 1.0f; // maps to [-1, 0.9998]
        grads[i]   = static_cast<float>(g_raw) / 5000.0f - 1.0f;
    }

    // print initial state
    std::cout << "Initial parameters:\n";
    for (int i = 0; i < DIM; ++i) {
        std::cout << weights[i] << (i + 1 == DIM ? '\n' : ' ');
    }

    // create optimizer (learning rate 0.01, beta1 0.9, beta2 0.999, epsilon 1e-8)
    AmsGrad optimiser(DIM, 0.01f, 0.9f, 0.999f, 1e-8f);

    // perform a few optimisation steps
    const int STEPS = 5;
    for (int s = 0; s < STEPS; ++s) {
        // in a real scenario grads would change each step;
        // here we keep them constant for demonstration.
        optimiser.step(weights, grads);
    }

    // print final state
    std::cout << "\nParameters after " << STEPS << " AmsGrad steps:\n";
    for (int i = 0; i < DIM; ++i) {
        std::cout << weights[i] << (i + 1 == DIM ? '\n' : ' ');
    }

    return 0;
}
