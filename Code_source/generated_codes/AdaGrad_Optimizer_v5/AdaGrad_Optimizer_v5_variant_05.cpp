#include <iostream>
#include <cmath>
#include <iomanip>

/* LLM input variant 5: duplicate-heavy */

// ------------------------------------------------------------
// Version #5 - AdaGrad Optimizer (verbose, stack‑arrays, class)
// ------------------------------------------------------------

#define DIM 5                // dimensionality of the problem (stack array size)
#define STEPS 8              // number of optimization iterations
#define LR 0.1f              // learning rate (step size)
#define EPS 1e-7f            // smoothing term to avoid division by zero

// -----------------------------------------------------------------
// Simple quadratic problem:
//   loss = 0.5 * Σ (param[i] - target[i])^2
//   gradient = param[i] - target[i]
// The target values are chosen to create edge‑case behaviour.
// -----------------------------------------------------------------
class AdaGrad {
public:
    // Constructor: initialise parameters and accumulator to zero
    AdaGrad() {
        // Verbose initialisation
        std::cout << "Initialising AdaGrad optimiser (stack arrays)" << std::endl;
        for (int idx = 0; idx < DIM; ++idx) {
            param[idx] = initParam[idx];      // start from a potentially problematic point
            accumSq[idx] = 0.0f;               // no accumulated gradient yet
        }
        std::cout << std::fixed << std::setprecision(6);
        printState("Initial state");
    }

    // Perform one optimisation step
    void step() {
        // Compute gradient for each dimension (verbose)
        for (int i = 0; i < DIM; ++i) {
            // gradient = param - target
            grad[i] = param[i] - target[i];
        }

        // Update accumulator with squared gradients (reordered arithmetic)
        int i = 0;
        while (i < DIM) {
            // accumSq += grad * grad   (square first, then add)
            float sq = grad[i] * grad[i];
            accumSq[i] = accumSq[i] + sq;
            ++i;
        }

        // Update parameters using AdaGrad rule
        i = 0;
        do {
            // denominator = sqrt(accumSq) + EPS  (reordered addition)
            float denom = sqrtf(accumSq[i]) + EPS;
            // effective step = (LR / denom) * grad   (division first, then multiplication)
            float eff = (LR / denom) * grad[i];
            // param := param - eff   (subtraction)
            param[i] = param[i] - eff;
            ++i;
        } while (i < DIM);
    }

    // Print current parameters, gradients and accumulator
    void printState(const char* label) {
        std::cout << "\n[" << label << "]" << std::endl;
        for (int j = 0; j < DIM; ++j) {
            std::cout << "  dim " << j
                      << " | param = " << param[j]
                      << " | grad = " << grad[j]
                      << " | accumSq = " << accumSq[j] << std::endl;
        }
    }

    // Access final parameters for external printing
    const float* getParams() const { return param; }

private:
    // Stack‑allocated arrays (no dynamic memory)
    float param[DIM];          // current parameters (weights)
    float grad[DIM];           // gradient of the loss w.r.t. each parameter
    float accumSq[DIM];        // accumulated sum of squared gradients

    // Fixed problem data (duplicate‑heavy)
    static const float initParam[DIM];
    static const float target[DIM];
};

// -----------------------------------------------------------------
// Definition of static problem data (duplicate‑heavy)
// -----------------------------------------------------------------
const float AdaGrad::initParam[DIM] = {
    1.0f,   // duplicate start value
    1.0f,   // duplicate start value
    -2.0f,  // duplicate negative start
    -2.0f,  // duplicate negative start
    0.0f    // zero start
};

const float AdaGrad::target[DIM] = {
    1.0f,   // same as init (zero gradient)
    1.0f,   // same as init (zero gradient)
    0.0f,   // shift from -2.0 to 0.0
    0.0f,   // shift from -2.0 to 0.0
    0.0f    // same as init (zero gradient)
};

// -----------------------------------------------------------------
// Main driver: runs the optimiser on the synthetic quadratic loss.
// -----------------------------------------------------------------
int main() {
    std::cout << "=== AdaGrad Optimiser Demo (Version #5) ===" << std::endl;

    AdaGrad optimiser;

    // Run a fixed number of steps, printing after each iteration
    for (int iter = 1; iter <= STEPS; ++iter) {
        std::cout << "\n--- Iteration " << iter << " ---" << std::endl;
        optimiser.step();                 // one optimisation step
        optimiser.printState("After step");
    }

    // Final outcome
    const float* finalParams = optimiser.getParams();
    std::cout << "\n=== Final Parameters ===" << std::endl;
    for (int k = 0; k < DIM; ++k) {
        std::cout << "param[" << k << "] = " << finalParams[k] << std::endl;
    }

    return 0;
}
