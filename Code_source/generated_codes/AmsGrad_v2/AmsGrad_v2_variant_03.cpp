#include <iostream>
#include <cmath>

/* LLM input variant 3: zeros-and-ones */

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
    // problem size
    int dim = 4;          // number of parameters
    int steps = 4;        // optimisation steps

    // allocate vectors on the heap
    float* w      = new float[dim];   // parameters
    float* m      = new float[dim];   // first‑moment
    float* v      = new float[dim];   // second‑moment
    float* v_max  = new float[dim];   // max of second‑moment
    float* grads  = new float[steps * dim]; // gradient history

    // initialise parameters and moments
    // mix of zeros and ones for initial parameters
    w[0] = 0.0f; w[1] = 1.0f; w[2] = 0.0f; w[3] = 1.0f;
    fill_array(m, dim, 0.0f);
    fill_array(v, dim, 0.0f);
    fill_array(v_max, dim, 0.0f);

    // predefined gradient sequence emphasizing zeros and ones
    // step 0
    grads[0] = 0.0f; grads[1] = 1.0f; grads[2] = 0.0f; grads[3] = 1.0f;
    // step 1
    grads[4] = 1.0f; grads[5] = 0.0f; grads[6] = 1.0f; grads[7] = 0.0f;
    // step 2 (all zeros)
    grads[8] = 0.0f; grads[9] = 0.0f; grads[10] = 0.0f; grads[11] = 0.0f;
    // step 3 (all ones)
    grads[12] = 1.0f; grads[13] = 1.0f; grads[14] = 1.0f; grads[15] = 1.0f;

    // hyper‑parameters (identity‑like values)
    float learning_rate = 1.0f;   // identity scaling
    float beta_one      = 0.0f;   // ignore previous first‑moment
    float beta_two      = 0.0f;   // ignore previous second‑moment
    float epsilon       = 1e-8f;  // small epsilon to avoid div‑zero

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
