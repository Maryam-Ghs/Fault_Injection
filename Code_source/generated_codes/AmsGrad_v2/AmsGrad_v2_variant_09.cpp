/* LLM input variant 9: medium-deterministic-random */
#include <iostream>
#include <cmath>

// ------------------------------------------------------------
// Helper: fill an array with a constant value
void fill_array(float* arr, int size, float val) {
    int i = 0;
    while (i < size) {
        arr[i] = val;
        ++i;
    }
}

// Helper: print a 1‑D float array
void show_array(const char* label, float* arr, int size) {
    std::cout << label;
    int i = 0;
    while (i < size) {
        std::cout << arr[i];
        if (i + 1 < size) std::cout << ", ";
        ++i;
    }
    std::cout << std::endl;
}

// AmsGrad single‑step update
void amsgrad_step(
    float* w,          // parameters (in‑place)
    float* g,          // current gradient
    float* m,          // first‑moment estimate
    float* v,          // second‑moment estimate
    float* v_max,      // max of second‑moment
    int    dim,        // dimension of vectors
    float  lr,         // learning rate
    float  b1,         // beta1
    float  b2,         // beta2
    float  eps)        // epsilon
{
    int j = 0;
    while (j < dim) {
        // update biased first moment
        m[j] = b1 * m[j] + (1.0f - b1) * g[j];

        // update biased second moment
        float grad_sq = g[j] * g[j];
        v[j] = b2 * v[j] + (1.0f - b2) * grad_sq;

        // maintain the maximum of v
        if (v[j] > v_max[j]) {
            v_max[j] = v[j];
        }

        // compute denominator once
        float denom = std::sqrt(v_max[j]) + eps;

        // parameter update
        w[j] = w[j] - lr * m[j] / denom;

        ++j;
    }
}

// ------------------------------------------------------------
int main() {
    // problem size (medium deterministic random)
    int dim   = 5;   // number of parameters
    int steps = 7;   // optimisation steps

    // allocate vectors on the heap
    float* w      = new float[dim];   // parameters
    float* m      = new float[dim];   // first‑moment
    float* v      = new float[dim];   // second‑moment
    float* v_max  = new float[dim];   // max of second‑moment
    float* grads  = new float[steps * dim]; // gradient history

    // initialise parameters and moments
    fill_array(w, dim, 0.5f);   // start at 0.5 for every param
    fill_array(m, dim, 0.0f);
    fill_array(v, dim, 0.0f);
    fill_array(v_max, dim, 0.0f);

    // deterministic pseudo‑random gradient sequence (size = steps * dim)
    static const float grad_vals[35] = {
        0.13f, -0.08f, 0.02f, -0.11f, 0.19f,
        0.04f, -0.06f, 0.15f, -0.02f, 0.07f,
        -0.09f, 0.12f, -0.03f, 0.05f, -0.14f,
        0.11f, -0.04f, 0.08f, -0.01f, 0.03f,
        -0.12f, 0.09f, -0.05f, 0.16f, -0.02f,
        0.07f, -0.08f, 0.14f, -0.03f, 0.06f,
        -0.01f, 0.02f, -0.07f, 0.09f, -0.05f
    };
    int total_vals = steps * dim;
    int i = 0;
    while (i < total_vals) {
        grads[i] = grad_vals[i];
        ++i;
    }

    // hyper‑parameters (float only)
    float learning_rate = 0.05f;
    float beta_one      = 0.9f;
    float beta_two      = 0.999f;
    float epsilon       = 1e-8f;

    // optimisation loop (while instead of for)
    int t = 0;
    while (t < steps) {
        float* cur_g = grads + t * dim;
        amsgrad_step(w, cur_g, m, v, v_max, dim,
                     learning_rate, beta_one, beta_two, epsilon);
        ++t;
    }

    // output final parameters
    show_array("Final parameters: ", w, dim);

    // clean up heap memory
    delete[] w;
    delete[] m;
    delete[] v;
    delete[] v_max;
    delete[] grads;

    return 0;
}
