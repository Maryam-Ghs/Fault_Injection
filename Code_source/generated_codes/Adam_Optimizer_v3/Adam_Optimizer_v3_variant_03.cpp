#include <iostream>
#include <cmath>   // only for std::sqrt, still works with float
#include <cstdlib> // for std::rand, std::srand
#include <ctime>   // for std::time

/* LLM input variant 3: zeros-and-ones */

// ------------------------------------------------------------
// Helper: allocate a float array on the heap and fill with a value
// ------------------------------------------------------------
float* allocate_and_fill(int size, float value)
{
    float* arr = new float[size];
    for (int i = 0; i < size; ++i)
        arr[i] = value;
    return arr;
}

// ------------------------------------------------------------
// Helper: compute a simple quadratic loss and its gradient
//          loss = Σ (w_i - t_i)^2
//          grad_i = 2 * (w_i - t_i)
// ------------------------------------------------------------
float compute_loss_and_gradient(const float* w, const float* t,
                                float* grad, int n)
{
    float loss = 0.0f;
    // manual unrolling for n == 4 (edge‑case heavy)
    loss += (w[0] - t[0]) * (w[0] - t[0]);
    grad[0] = 2.0f * (w[0] - t[0]);

    loss += (w[1] - t[1]) * (w[1] - t[1]);
    grad[1] = 2.0f * (w[1] - t[1]);

    loss += (w[2] - t[2]) * (w[2] - t[2]);
    grad[2] = 2.0f * (w[2] - t[2]);

    loss += (w[3] - t[3]) * (w[3] - t[3]);
    grad[3] = 2.0f * (w[3] - t[3]);

    return loss;
}

// ------------------------------------------------------------
// Helper: single Adam step (manual unrolled loop)
// ------------------------------------------------------------
void adam_step(float* w, float* m, float* v,
               const float* grad, int n,
               int step,
               float lrate, float b1, float b2, float eps)
{
    // ---- first moment update ----
    m[0] = b1 * m[0] + (1.0f - b1) * grad[0];
    m[1] = b1 * m[1] + (1.0f - b1) * grad[1];
    m[2] = b1 * m[2] + (1.0f - b1) * grad[2];
    m[3] = b1 * m[3] + (1.0f - b1) * grad[3];

    // ---- second moment update ----
    v[0] = b2 * v[0] + (1.0f - b2) * grad[0] * grad[0];
    v[1] = b2 * v[1] + (1.0f - b2) * grad[1] * grad[1];
    v[2] = b2 * v[2] + (1.0f - b2) * grad[2] * grad[2];
    v[3] = b2 * v[3] + (1.0f - b2) * grad[3] * grad[3];

    // ---- bias‑corrected moments (expanded multi‑step) ----
    float pow_b1 = 1.0f;
    float pow_b2 = 1.0f;
    for (int i = 0; i < step; ++i) {
        pow_b1 *= b1;
        pow_b2 *= b2;
    }
    float m_corr0 = m[0] / (1.0f - pow_b1);
    float m_corr1 = m[1] / (1.0f - pow_b1);
    float m_corr2 = m[2] / (1.0f - pow_b1);
    float m_corr3 = m[3] / (1.0f - pow_b1);
    float v_corr0 = v[0] / (1.0f - pow_b2);
    float v_corr1 = v[1] / (1.0f - pow_b2);
    float v_corr2 = v[2] / (1.0f - pow_b2);
    float v_corr3 = v[3] / (1.0f - pow_b2);

    // ---- parameter update (reordered mathematically) ----
    float denom0 = std::sqrt(v_corr0) + eps;
    float denom1 = std::sqrt(v_corr1) + eps;
    float denom2 = std::sqrt(v_corr2) + eps;
    float denom3 = std::sqrt(v_corr3) + eps;

    w[0] = w[0] - lrate * (m_corr0 / denom0);
    w[1] = w[1] - lrate * (m_corr1 / denom1);
    w[2] = w[2] - lrate * (m_corr2 / denom2);
    w[3] = w[3] - lrate * (m_corr3 / denom3);
}

// ------------------------------------------------------------
// Main driver – generates edge‑case heavy data, runs Adam, prints
// ------------------------------------------------------------
int main()
{
    // Seed for reproducibility (edge‑case: fixed seed)
    std::srand(42);

    const int DIM = 4;          // fixed size for manual unrolling
    const int ITER = 6;         // reduced iterations for quick convergence

    // Hyper‑parameters emphasizing zeros and ones
    float learning_rate = 1.0f; // identity learning rate
    float beta1 = 0.0f;         // no first‑moment decay
    float beta2 = 0.0f;         // no second‑moment decay
    float epsilon = 1e-8f;

    // Allocate parameters, moments, gradient, target
    float* weight = allocate_and_fill(DIM, 0.0f);
    float* first_m = allocate_and_fill(DIM, 0.0f);
    float* second_v = allocate_and_fill(DIM, 0.0f);
    float* gradient = allocate_and_fill(DIM, 0.0f);
    float* target = new float[DIM];

    // Zero‑and‑one heavy target
    target[0] = 0.0f;
    target[1] = 1.0f;
    target[2] = 0.0f;
    target[3] = 1.0f;

    // Run Adam optimizer
    for (int step = 1; step <= ITER; ++step)
    {
        // Compute loss and gradient (expanded multi‑step inside)
        float loss = compute_loss_and_gradient(weight, target, gradient, DIM);

        // Perform Adam update (manual unrolled)
        adam_step(weight, first_m, second_v, gradient, DIM,
                  step, learning_rate, beta1, beta2, epsilon);

        // Print status
        std::cout << "Iter " << step << " | Loss = " << loss << "\n";
        std::cout << "  w: ["
                  << weight[0] << ", "
                  << weight[1] << ", "
                  << weight[2] << ", "
                  << weight[3] << "]\n";
    }

    // Final result
    std::cout << "\nFinal parameters after " << ITER << " iterations:\n";
    for (int i = 0; i < DIM; ++i)
        std::cout << "w[" << i << "] = " << weight[i] << "\n";

    // Clean up heap memory
    delete[] weight;
    delete[] first_m;
    delete[] second_v;
    delete[] gradient;
    delete[] target;

    return 0;
}
