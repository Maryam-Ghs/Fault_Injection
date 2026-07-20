/* LLM input variant 4: signed-extremes */
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
    // Edge‑case heavy configuration with mixed signed extremes
    int dimension = 6;                // increased dimension to showcase more cases
    float learning_rate = 5.0f;        // moderate learning rate
    float epsilon = 1e-6f;             // small epsilon to keep stability

    // Create optimizer instance
    AdaGrad opt(dimension, learning_rate, epsilon);

    // Allocate gradient vector on the heap
    float *grad = new float[dimension];

    // Populate gradient with a mixture of zero, small, and large signed values
    grad[0] = 0.0f;          // zero gradient
    grad[1] = -0.75f;        // small negative
    grad[2] = 2.5f;         // small positive
    grad[3] = -8.0e3f;       // large negative
    grad[4] = 8.0e3f;        // large positive
    grad[5] = -3.4e3f;       // medium negative

    // Run several optimisation steps
    int steps = 4;
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
