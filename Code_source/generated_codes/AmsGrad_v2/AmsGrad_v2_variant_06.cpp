#include <iostream>
#include <cmath>

/* LLM input variant 6: ordered-structured */
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
    // problem size (ordered, regular dimensions)
    int dim = 4;          // number of parameters
    int steps = 4;        // optimisation steps

    // allocate vectors on the heap
    float* w      = new float[dim];   // parameters
    float* m      = new float[dim];   // first‑moment
    float* v      = new float[dim];   // second‑moment
    float* v_max  = new float[dim];   // max of second‑moment
    float* grads  = new float[steps * dim]; // gradient history

    // initialise parameters and moments
    fill_array(w, dim, 2.0f);         // start at 2.0 for every param (ordered start)
    fill_array(m, dim, 0.0f);
    fill_array(v, dim, 0.0f);
    fill_array(v_max, dim, 0.0f);

    // highly structured, monotonic gradient sequence (sorted ascending)
    // step 0
    grads[0] = -0.20f; grads[1] = -0.10f; grads[2] = 0.00f; grads[3] = 0.10f;
    // step 1
    grads[4] = -0.05f; grads[5] = 0.00f; grads[6] = 0.05f; grads[7] = 0.10f;
    // step 2
    grads[8] = 0.00f; grads[9] = 0.05f; grads[10] = 0.10f; grads[11] = 0.15f;
    // step 3
    grads[12] = 0.05f; grads[13] = 0.10f; grads[14] = 0.15f; grads[15] = 0.20f;

    // hyper‑parameters (float only)
    float learning_rate = 0.1f;
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
