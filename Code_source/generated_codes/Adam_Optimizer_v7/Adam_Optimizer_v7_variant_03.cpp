/* LLM input variant 3: zeros-and-ones */
// Adam Optimizer implementation – version #7
// ------------------------------------------------
// Uses only float and int types, stack‑allocated arrays,
// class‑based design, and modular functions.
// ------------------------------------------------

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>

#define DIM 10               // dimensionality of the problem
#define STEPS 500            // number of Adam updates

// ------------------------------------------------
// Helper: generate a random float in [-5,5]
// ------------------------------------------------
float rand_float()
{
    return (float)std::rand() / (float)RAND_MAX * 10.0f - 5.0f;
}

// ------------------------------------------------
// Compute gradient of simple quadratic loss:
//   L(w) = Σ (w_i - tgt_i)^2
// Gradient: 2 * (w_i - tgt_i)
// ------------------------------------------------
void calc_grad(float w[DIM], float tgt[DIM], float g[DIM])
{
    for (int i = 0; i < DIM; ++i)
        g[i] = 2.0f * (w[i] - tgt[i]);
}

// ------------------------------------------------
// Adam optimizer class (stack state)
// ------------------------------------------------
class AdamOpt
{
public:
    float step_sz;      // learning rate
    float b1;           // decay for first moment
    float b2;           // decay for second moment
    float eps;          // small constant
    float m[DIM];       // first moment vector
    float v[DIM];       // second moment vector
    int   cnt;          // time step

    // Constructor – initialise all moments to zero
    AdamOpt(float lr, float beta1, float beta2, float epsilon)
    {
        step_sz = lr;
        b1      = beta1;
        b2      = beta2;
        eps     = epsilon;
        cnt     = 0;
        for (int i = 0; i < DIM; ++i)
        {
            m[i] = 0.0f;
            v[i] = 0.0f;
        }
    }

    // Perform one Adam update
    void update(float w[DIM], float g[DIM])
    {
        ++cnt;   // increment time step

        // bias‑corrected learning rate (reordered computation)
        float lr_t = step_sz *
                     sqrtf(1.0f - powf(b2, (float)cnt)) /
                     (1.0f - powf(b1, (float)cnt));

        // iterate backwards – a valid re‑ordering
        for (int i = DIM - 1; i >= 0; --i)
        {
            m[i] = b1 * m[i] + (1.0f - b1) * g[i];
            v[i] = b2 * v[i] + (1.0f - b2) * g[i] * g[i];
            w[i] -= lr_t * m[i] / (sqrtf(v[i]) + eps);
        }
    }
};

// ------------------------------------------------
// Main driver
// ------------------------------------------------
int main()
{
    std::srand((unsigned)std::time(nullptr));

    // --- initialise problem data on the stack ---
    float target[DIM];
    float weight[DIM];
    float grad[DIM];

    // Emphasize zeros and ones: target is all zeros,
    // weight alternates between 1 and 0.
    for (int i = 0; i < DIM; ++i)
    {
        target[i] = 0.0f;                     // desired values (zeros)
        weight[i] = (i % 2 == 0) ? 1.0f : 0.0f; // starting parameters (ones and zeros)
    }

    // --- create Adam optimiser instance ---
    AdamOpt optimizer(0.05f, 0.9f, 0.999f, 1e-8f);

    // --- run optimisation loop ---
    for (int step = 0; step < STEPS; ++step)
    {
        calc_grad(weight, target, grad);
        optimizer.update(weight, grad);
    }

    // --- output final parameters ---
    std::cout << "Final parameters after " << STEPS << " Adam steps:\n";
    for (int i = 0; i < DIM; ++i)
        std::cout << "w[" << i << "] = " << weight[i] << "\n";

    return 0;
}
