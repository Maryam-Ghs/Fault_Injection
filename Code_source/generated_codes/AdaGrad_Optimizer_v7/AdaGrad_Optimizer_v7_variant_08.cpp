/* LLM input variant 8: sparse-skewed */
// AdaGrad Optimizer – version #7
// Implements a simple AdaGrad with heap‑allocated arrays, modular functions,
// temporary variables for each arithmetic step, and edge‑case heavy inputs.

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
    // Edge‑case heavy configuration with a large, mostly empty gradient
    int dimension = 1000;           // high dimension to expose sparsity
    float learning_rate = 10.0f;   // unusually large learning rate
    float epsilon = 1e-8f;          // very small epsilon

    // Create optimizer instance
    AdaGrad opt(dimension, learning_rate, epsilon);

    // Allocate gradient vector on the heap
    float *grad = new float[dimension];

    // Initialise all gradients to zero (sparse input)
    int init_idx = 0;
    while (init_idx < dimension) {
        grad[init_idx] = 0.0f;
        ++init_idx;
    }

    // Populate a few non‑zero gradients at widely separated indices
    grad[10]   = 1.0f;          // small positive
    grad[20]   = -0.5f;         // small negative
    grad[500]  = 5.0e3f;        // huge positive
    grad[999]  = -5.0e3f;       // huge negative

    // Run several optimisation steps
    int steps = 3;
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
