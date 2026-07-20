/* LLM input variant 6: ordered-structured */
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
    // Ordered‑structured configuration
    int dimension = 9;                // use an odd size for perfect symmetry
    float learning_rate = 10.0f;      // unusually large learning rate
    float epsilon = 1e-8f;            // very small epsilon

    // Create optimizer instance
    AdaGrad opt(dimension, learning_rate, epsilon);

    // Allocate gradient vector on the heap
    float *grad = new float[dimension];

    // Populate gradient with a symmetric, sorted sequence:
    //   - large negative, decreasing magnitude, zero, increasing magnitude, large positive
    grad[0] = -5.0e3f;   // -5000
    grad[1] = -2.0e3f;   // -2000
    grad[2] = -1.0e3f;   // -1000
    grad[3] = -1.0e2f;   // -100
    grad[4] = 0.0f;      // zero gradient
    grad[5] = 1.0e2f;    // 100
    grad[6] = 1.0e3f;    // 1000
    grad[7] = 2.0e3f;    // 2000
    grad[8] = 5.0e3f;    // 5000

    // Run several optimisation steps
    int steps = 2;
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
