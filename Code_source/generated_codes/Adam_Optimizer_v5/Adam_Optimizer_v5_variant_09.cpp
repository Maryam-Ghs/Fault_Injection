/* LLM input variant 9: medium-deterministic-random */
// Adam Optimizer – Version 5
// Implements a simple Adam update on randomly generated data.
// Uses only float/int, heap allocation, manual loop unrolling and temporary variables.

#include <iostream>
#include <cmath>

// Simple linear congruential generator for reproducible pseudo‑random numbers
struct RandGen {
    int seed;
    RandGen(int s) : seed(s) {}
    // returns a float in [-1,1)
    float next() {
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        return (float(seed) / 1073741824.0f) - 1.0f;
    }
};

// ------------------------------------------------------------
// Adam optimizer class (heap‑based state)
class AdamUpd {
public:
    // hyper‑parameters
    float lr;      // learning rate
    float b1;      // beta1
    float b2;      // beta2
    float eps;    // epsilon

    // moment vectors
    float* m_vec;
    float* v_vec;

    // time step counter
    int step;
    // dimensionality
    int dim;

    AdamUpd(int d, float a_lr = 0.001f, float a_b1 = 0.9f,
            float a_b2 = 0.999f, float a_eps = 1e-8f)
        : lr(a_lr), b1(a_b1), b2(a_b2), eps(a_eps),
          step(0), dim(d)
    {
        // allocate moments on the heap and zero‑initialize
        m_vec = new float[dim];
        v_vec = new float[dim];
        for (int i = 0; i < dim; ++i) {
            m_vec[i] = 0.0f;
            v_vec[i] = 0.0f;
        }
    }

    ~AdamUpd() {
        delete[] m_vec;
        delete[] v_vec;
    }

    // single Adam update (in‑place on param array)
    void step_update(float* param, const float* grad) {
        ++step;                                   // increment time step
        float b1_pow = std::pow(b1, float(step));
        float b2_pow = std::pow(b2, float(step));
        float lr_corr = lr * std::sqrt(1.0f - b2_pow) / (1.0f - b1_pow);

        // manual unrolling: process four elements per loop iteration
        int i = 0;
        for (; i + 3 < dim; i += 4) {
            // ---- element i ----
            float g0 = grad[i];
            float m0 = b1 * m_vec[i] + (1.0f - b1) * g0;
            float v0 = b2 * v_vec[i] + (1.0f - b2) * g0 * g0;
            float m_hat0 = m0 / (1.0f - b1_pow);
            float v_hat0 = v0 / (1.0f - b2_pow);
            float delta0 = lr_corr * m_hat0 / (std::sqrt(v_hat0) + eps);
            param[i] -= delta0;
            m_vec[i] = m0;
            v_vec[i] = v0;

            // ---- element i+1 ----
            float g1 = grad[i + 1];
            float m1 = b1 * m_vec[i + 1] + (1.0f - b1) * g1;
            float v1 = b2 * v_vec[i + 1] + (1.0f - b2) * g1 * g1;
            float m_hat1 = m1 / (1.0f - b1_pow);
            float v_hat1 = v1 / (1.0f - b2_pow);
            float delta1 = lr_corr * m_hat1 / (std::sqrt(v_hat1) + eps);
            param[i + 1] -= delta1;
            m_vec[i + 1] = m1;
            v_vec[i + 1] = v1;

            // ---- element i+2 ----
            float g2 = grad[i + 2];
            float m2 = b1 * m_vec[i + 2] + (1.0f - b1) * g2;
            float v2 = b2 * v_vec[i + 2] + (1.0f - b2) * g2 * g2;
            float m_hat2 = m2 / (1.0f - b1_pow);
            float v_hat2 = v2 / (1.0f - b2_pow);
            float delta2 = lr_corr * m_hat2 / (std::sqrt(v_hat2) + eps);
            param[i + 2] -= delta2;
            m_vec[i + 2] = m2;
            v_vec[i + 2] = v2;

            // ---- element i+3 ----
            float g3 = grad[i + 3];
            float m3 = b1 * m_vec[i + 3] + (1.0f - b1) * g3;
            float v3 = b2 * v_vec[i + 3] + (1.0f - b2) * g3 * g3;
            float m_hat3 = m3 / (1.0f - b1_pow);
            float v_hat3 = v3 / (1.0f - b2_pow);
            float delta3 = lr_corr * m_hat3 / (std::sqrt(v_hat3) + eps);
            param[i + 3] -= delta3;
            m_vec[i + 3] = m3;
            v_vec[i + 3] = v3;
        }

        // tail loop for remaining elements
        for (; i < dim; ++i) {
            float g = grad[i];
            float m = b1 * m_vec[i] + (1.0f - b1) * g;
            float v = b2 * v_vec[i] + (1.0f - b2) * g * g;
            float m_hat = m / (1.0f - b1_pow);
            float v_hat = v / (1.0f - b2_pow);
            float delta = lr_corr * m_hat / (std::sqrt(v_hat) + eps);
            param[i] -= delta;
            m_vec[i] = m;
            v_vec[i] = v;
        }
    }
};

// ------------------------------------------------------------
int main() {
    // problem size – medium random vectors with a different layout
    const int DIM = 23;                 // odd to keep tail loop exercised
    float* weights = new float[DIM];
    float* grads   = new float[DIM];

    // generate deterministic pseudo‑random data with a new seed
    RandGen rng(98765);
    for (int i = 0; i < DIM; ++i) {
        weights[i] = rng.next();        // initial parameters in [-1,1)
        grads[i]   = rng.next();        // mock gradients in same range
    }

    // instantiate optimizer with varied hyper‑parameters
    AdamUpd optimizer(DIM, 0.005f, 0.85f, 0.98f, 1e-7f);

    // run a few more update steps
    const int STEPS = 7;
    for (int s = 0; s < STEPS; ++s) {
        // pretend new gradients each step (regenerate)
        for (int i = 0; i < DIM; ++i)
            grads[i] = rng.next();

        optimizer.step_update(weights, grads);
    }

    // print final parameters
    std::cout << "Final parameters after " << STEPS << " Adam steps:\n";
    for (int i = 0; i < DIM; ++i) {
        std::cout << weights[i] << (i + 1 == DIM ? '\n' : ' ');
    }

    // clean up heap memory
    delete[] weights;
    delete[] grads;
    return 0;
}
