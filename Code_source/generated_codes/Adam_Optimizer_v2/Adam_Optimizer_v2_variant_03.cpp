/* LLM input variant 3: zeros-and-ones */
#include <iostream>
#include <cmath>

#define DIM 3          // dimensionality of the parameter vector
#define STEPS 1000     // number of optimisation steps

// ------------------------------------------------------------
// Simple Adam optimiser (float only, stack‑allocated state)
// ------------------------------------------------------------
class AdamSolver {
public:
    // hyper‑parameters (all floats, no const)
    float lr;          // base learning rate
    float b1;          // exponential decay rate for first moment
    float b2;          // exponential decay rate for second moment
    float eps;         // small constant to avoid division by zero

    // internal state (stack arrays)
    float m[DIM];
    float v[DIM];
    float beta1_pow;
    float beta2_pow;

    // --------------------------------------------------------
    // constructor – initialise hyper‑parameters and moments
    // --------------------------------------------------------
    AdamSolver(float learn, float beta_one, float beta_two, float epsilon) {
        lr   = learn;
        b1   = beta_one;
        b2   = beta_two;
        eps  = epsilon;

        // zero‑initialise moments
        for (int i = 0; i < DIM; ++i) {
            m[i] = 0.0f;
            v[i] = 0.0f;
        }
        beta1_pow = 1.0f;
        beta2_pow = 1.0f;
    }

    // --------------------------------------------------------
    // single optimisation step
    // --------------------------------------------------------
    void step(float param[DIM], const float grad[DIM]) {
        // update biased first and second moments
        for (int i = 0; i < DIM; ++i) {
            m[i] = b1 * m[i] + (1.0f - b1) * grad[i];
            v[i] = b2 * v[i] + (1.0f - b2) * grad[i] * grad[i];
        }

        // update bias‑correction factors
        beta1_pow *= b1;
        beta2_pow *= b2;

        // compute bias‑corrected learning rate once per step
        float lr_t = lr * std::sqrt(1.0f - beta2_pow) / (1.0f - beta1_pow);

        // apply parameter update (order of operations rearranged)
        for (int i = 0; i < DIM; ++i) {
            float denom = std::sqrt(v[i]) + eps;
            float step_size = lr_t * m[i] / denom;
            param[i] -= step_size;                 // note: subtraction after division
        }
    }
};

// ------------------------------------------------------------
// Simple quadratic loss:  L(w) = Σ (w_i - target_i)^2
// Its gradient:          g_i = 2 * (w_i - target_i)
// ------------------------------------------------------------
void compute_gradient(const float w[DIM], const float target[DIM], float out_grad[DIM]) {
    for (int i = 0; i < DIM; ++i) {
        out_grad[i] = 2.0f * (w[i] - target[i]);
    }
}

// ------------------------------------------------------------
// Main driver – deterministic test vector, loop‑heavy style
// ------------------------------------------------------------
int main() {
    // deterministic initial parameters (all zeros)
    float theta[DIM] = {0.0f, 0.0f, 0.0f};

    // deterministic target vector emphasizing zeros and ones
    float goal[DIM] = {1.0f, 0.0f, 1.0f};

    // Adam hyper‑parameters chosen to highlight zeros (beta terms) and identity learning rate
    float learning_rate = 1.0f;
    float beta_one      = 0.0f;
    float beta_two      = 0.0f;
    float epsilon       = 1e-8f;

    // instantiate optimiser
    AdamSolver optimizer(learning_rate, beta_one, beta_two, epsilon);

    // buffer for gradient (stack‑allocated)
    float grad[DIM];

    // iterative optimisation loop (loop‑heavy)
    for (int step = 0; step < STEPS; ++step) {
        // compute current gradient
        compute_gradient(theta, goal, grad);

        // perform Adam update
        optimizer.step(theta, grad);
    }

    // print final parameters
    std::cout << "Optimised parameters after " << STEPS << " steps:\n";
    for (int i = 0; i < DIM; ++i) {
        std::cout << "theta[" << i << "] = " << theta[i] << '\n';
    }

    // also print the target for visual confirmation
    std::cout << "\nTarget vector:\n";
    for (int i = 0; i < DIM; ++i) {
        std::cout << "goal[" << i << "] = " << goal[i] << '\n';
    }

    return 0;
}
