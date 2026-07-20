#include <iostream>
#include <cmath>
#include <iomanip>

/* LLM input variant 1: minimal-boundary */

// ------------------------------------------------------------
// Version #5 - AdaGrad Optimizer (verbose, stack‑arrays, class)
// ------------------------------------------------------------

#define DIM 1                // minimal dimensionality (single variable)
#define STEPS 1              // minimal number of optimization iterations
#define LR 0.1f              // learning rate (step size)
#define EPS 1e-7f            // smoothing term to avoid division by zero

// -----------------------------------------------------------------
// Simple quadratic problem:
//   loss = 0.5 * Σ (param[i] - target[i])^2
//   gradient = param[i] - target[i]
// -----------------------------------------------------------------
class AdaGrad {
public:
    AdaGrad() {
        std::cout << "Initialising AdaGrad optimiser (stack arrays)" << std::endl;
        for (int idx = 0; idx < DIM; ++idx) {
            param[idx] = initParam[idx];
            accumSq[idx] = 0.0f;
        }
        std::cout << std::fixed << std::setprecision(6);
        printState("Initial state");
    }

    void step() {
        for (int i = 0; i < DIM; ++i) {
            grad[i] = param[i] - target[i];
        }

        int i = 0;
        while (i < DIM) {
            float sq = grad[i] * grad[i];
            accumSq[i] = accumSq[i] + sq;
            ++i;
        }

        i = 0;
        do {
            float denom = sqrtf(accumSq[i]) + EPS;
            float eff = (LR / denom) * grad[i];
            param[i] = param[i] - eff;
            ++i;
        } while (i < DIM);
    }

    void printState(const char* label) {
        std::cout << "\n[" << label << "]" << std::endl;
        for (int j = 0; j < DIM; ++j) {
            std::cout << "  dim " << j
                      << " | param = " << param[j]
                      << " | grad = " << grad[j]
                      << " | accumSq = " << accumSq[j] << std::endl;
        }
    }

    const float* getParams() const { return param; }

private:
    float param[DIM];
    float grad[DIM];
    float accumSq[DIM];

    static const float initParam[DIM];
    static const float target[DIM];
};

// -----------------------------------------------------------------
// Definition of static problem data (minimal)
// -----------------------------------------------------------------
const float AdaGrad::initParam[DIM] = { 0.0f };
const float AdaGrad::target[DIM] = { 1.0f };

int main() {
    std::cout << "=== AdaGrad Optimiser Demo (Version #5) ===" << std::endl;

    AdaGrad optimiser;

    for (int iter = 1; iter <= STEPS; ++iter) {
        std::cout << "\n--- Iteration " << iter << " ---" << std::endl;
        optimiser.step();
        optimiser.printState("After step");
    }

    const float* finalParams = optimiser.getParams();
    std::cout << "\n=== Final Parameters ===" << std::endl;
    for (int k = 0; k < DIM; ++k) {
        std::cout << "param[" << k << "] = " << finalParams[k] << std::endl;
    }

    return 0;
}
