// AdaGrad Optimizer – version #7
// Implements a simple AdaGrad with heap‑allocated arrays, modular functions,
// temporary variables for each arithmetic step, and edge‑case heavy inputs.

/* LLM input variant 9: medium-deterministic-random */

#include <iostream>
#include <cmath>

class AdaGrad {
public:
    int dim;        // number of parameters
    float *w;       // parameters (weights)
    float *g2;      // accumulated squared gradients
    float lr;       // base learning rate
    float eps;      // epsilon to avoid division by zero

    // Constructor: allocate arrays on the heap and initialise them
    AdaGrad(int d, float learning_rate, float epsilon) {
        dim = d;
        lr = learning_rate;
        eps = epsilon;
        w = new float[dim];
        g2 = new float[dim];
        int idx = 0;
        while (idx < dim) {
            w[idx] = 0.0f;      // start from zero vector
            g2[idx] = 0.0f;     // no accumulated gradient yet
            ++idx;
        }
    }

    // Destructor: free heap memory
    ~AdaGrad() {
        delete[] w;
        delete[] g2;
    }

    // Single optimisation step given a gradient vector
    void step(float *grad) {
        int i = 0;
        while (i < dim) {
            // ---- read gradient -------------------------------------------------
            float g_val = grad[i];

            // ---- accumulate squared gradient -----------------------------------
            float sq = g_val * g_val;
            float acc = g2[i] + sq;
            g2[i] = acc;

            // ---- compute adjusted learning rate --------------------------------
            float sqrt_acc = sqrtf(acc);
            float denom = sqrt_acc + eps;
            float adj_lr = lr / denom;

            // ---- parameter update -----------------------------------------------
            float delta = -adj_lr * g_val;
            float new_w = w[i] + delta;
            w[i] = new_w;

            ++i;
        }
    }

    // Print current parameters
    void print() {
        int i = 0;
        while (i < dim) {
            std::cout << w[i];
            if (i + 1 < dim) std::cout << ' ';
            ++i;
        }
        std::cout << std::endl;
    }
};

int main() {
    // Medium‑size deterministic input configuration
    int dimension = 12;          // medium dimension to provide diverse cases
    float learning_rate = 2.5f;  // moderate learning rate
    float epsilon = 1e-5f;        // small epsilon to avoid division by zero

    // Create optimizer instance
    AdaGrad opt(dimension, learning_rate, epsilon);

    // Allocate gradient vector on the heap
    float *grad = new float[dimension];

    // Deterministic pseudo‑random‑looking gradient values covering various cases
    grad[0] = 0.0f;           // zero gradient
    grad[1] = 13.5f;          // moderate positive
    grad[2] = -7.2f;          // moderate negative
    grad[3] = 5.0e3f;         // huge positive
    grad[4] = -4.0e3f;        // huge negative
    grad[5] = 0.001f;        // tiny positive
    grad[6] = -0.001f;       // tiny negative
    grad[7] = 123.456f;       // medium positive with fractional part
    grad[8] = -987.654f;      // medium negative with fractional part
    grad[9] = 1.0e-4f;        // very small positive
    grad[10] = -1.0e4f;       // large negative
    grad[11] = 42.0f;         // arbitrary positive

    // Run several optimisation steps
    int steps = 5;
    int s = 0;
    while (s < steps) {
        opt.step(grad);
        ++s;
    }

    // Output final parameters
    opt.print();

    // Clean up
    delete[] grad;
    return 0;
}
