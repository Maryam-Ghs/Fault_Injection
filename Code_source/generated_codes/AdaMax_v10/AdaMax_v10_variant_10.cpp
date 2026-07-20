// AdaMax optimizer – version #10
// Implements AdaMax on a randomly‑generated float array.
// Uses only float and int, stack‑allocated arrays and modular helper functions.
/* LLM input variant 10: large-safe-stress */

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>    // for fabs and fmaxf

// ------------------------------------------------------------
// Fill a stack array with uniform random numbers in [-1, 1]
void fill_random(float arr[], int len)
{
    for (int i = 0; i < len; ++i)
    {
        // rand() returns int; cast to float and scale.
        float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        arr[i] = 2.0f * r - 1.0f;          // map to [-1, 1]
    }
}

// ------------------------------------------------------------
// Compute β1^t by repeated multiplication (avoids std::pow)
float pow_beta1(float beta1, int t)
{
    float result = 1.0f;
    int cnt = 0;
    while (cnt < t)
    {
        result *= beta1;
        ++cnt;
    }
    return result;
}

// ------------------------------------------------------------
// One AdaMax step for the whole vector
void ada_max_step(
    float parm[],          // parameters (w)
    const float grad[],    // gradients (g)
    float moment[],        // first moment (m)
    float inf_norm[],      // infinity norm (u)
    int size,
    float learn,           // α
    float b1,              // β1
    float b2,              // β2
    float eps,             // ε
    int step_idx)          // t (1‑based)
{
    // -------------------- pre‑compute bias‑correction factor
    float beta_pow = pow_beta1(b1, step_idx);
    float bias_corr = 1.0f - beta_pow;          // 1‑β1^t

    // -------------------- element‑wise updates (while loop style)
    int idx = 0;
    while (idx < size)
    {
        // ----- temp variables for readability
        float g_val   = grad[idx];
        float m_prev  = moment[idx];
        float u_prev  = inf_norm[idx];

        // ----- update first moment m_t
        float m_tmp   = b1 * m_prev + (1.0f - b1) * g_val;

        // ----- update infinity norm u_t
        float abs_g   = fabs(g_val);
        float u_tmp   = fmaxf(b2 * u_prev, abs_g);

        // ----- bias‑corrected first moment
        float m_hat   = m_tmp / bias_corr;

        // ----- compute step size
        float denom   = u_tmp + eps;
        float step    = learn * m_hat / denom;

        // ----- apply parameter update
        parm[idx]    -= step;

        // ----- store back updated moments
        moment[idx]   = m_tmp;
        inf_norm[idx] = u_tmp;

        ++idx;
    }
}

// ------------------------------------------------------------
// Print a float array with a label
void show_array(const char *label, const float arr[], int len)
{
    std::cout << label << ": [ ";
    for (int i = 0; i < len; ++i)
    {
        std::cout << arr[i];
        if (i + 1 < len) std::cout << ", ";
    }
    std::cout << " ]\n";
}

// ------------------------------------------------------------
int main()
{
    // ---- configuration -------------------------------------------------
    const int N        = 1000;       // vector length (stack array) – larger safe size
    const int STEPS    = 200;        // number of AdaMax iterations – increased for stress
    const float LR     = 0.02f;    // learning rate α
    const float B1     = 0.9f;     // β1
    const float B2     = 0.999f;   // β2 (for infinity norm)
    const float EPS    = 1e-8f;    // ε to avoid division by zero

    // ---- stack‑allocated vectors ---------------------------------------
    float weight[N];               // parameters w
    float grad[N];                 // synthetic gradients g
    float m_vec[N];                // first moment m
    float u_vec[N];                // infinity norm u

    // ---- generate deterministic initial data ----------------------------------
    std::srand(42); // fixed seed for reproducibility
    fill_random(weight, N);
    fill_random(grad,   N);
    // initialise moments to zero
    for (int i = 0; i < N; ++i)
    {
        m_vec[i] = 0.0f;
        u_vec[i] = 0.0f;
    }

    // ---- show starting point -------------------------------------------
    show_array("Initial weights", weight, N);

    // ---- run AdaMax -----------------------------------------------------
    int t = 1;
    while (t <= STEPS)
    {
        ada_max_step(weight, grad, m_vec, u_vec,
                     N, LR, B1, B2, EPS, t);
        ++t;
    }

    // ---- final result ---------------------------------------------------
    show_array("Final weights", weight, N);

    return 0;
}
