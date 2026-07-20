#include <iostream>
#include <cmath>

/* LLM input variant 7: reverse-adversarial */

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
    // problem size (increased dimension for adversarial stress)
    int dim = 5;          // number of parameters
    int steps = 5;        // optimisation steps

    // allocate vectors on the heap
    float* w      = new float[dim];   // parameters
    float* m      = new float[dim];   // first‑moment
    float* v      = new float[dim];   // second‑moment
    float* v_max  = new float[dim];   // max of second‑moment
    float* grads  = new float[steps * dim]; // gradient history

    // initialise parameters and moments with large magnitude
    fill_array(w, dim, 1000.0f);   // start far from optimum
    fill_array(m, dim, 0.0f);
    fill_array(v, dim, 0.0f);
    fill_array(v_max, dim, 0.0f);

    // adversarial gradient sequence: reverse‑ordered, decreasing magnitudes,
    // alternating signs to provoke worst‑case behaviour
    // step 0 (largest magnitude)
    grads[0] = 10.0f; grads[1] = -9.5f; grads[2] = 9.0f; grads[3] = -8.5f; grads[4] = 8.0f;
    // step 1
    grads[5] = -7.5f; grads[6] = 7.0f; grads[7] = -6.5f; grads[8] = 6.0f; grads[9] = -5.5f;
    // step 2
    grads[10] = 5.0f; grads[11] = -4.5f; grads[12] = 4.0f; grads[13] = -3.5f; grads[14] = 3.0f;
    // step 3
    grads[15] = -2.5f; grads[16] = 2.0f; grads[17] = -1.5f; grads[18] = 1.0f; grads[19] = -0.5f;
    // step 4 (smallest magnitude)
    grads[20] = 0.25f; grads[21] = -0.2f; grads[22] = 0.15f; grads[23] = -0.1f; grads[24] = 0.05f;

    // hyper‑parameters (float only)
    float learning_rate = 0.01f;   // smaller LR to keep updates stable with large grads
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
