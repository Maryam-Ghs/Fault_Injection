#include <iostream>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <ctime>
/* LLM input variant 1: minimal-boundary */

int main() {
    // ------------------------------------------------------------
    // Seed the pseudo‑random generator (deterministic for testing)
    // ------------------------------------------------------------
    std::srand(0);

    // ------------------------------------------------------------
    // Hyper‑parameters for Adadelta (float only)
    // ------------------------------------------------------------
    float rho = 0.95f;          // decay factor
    float eps = 1e-6f;          // numerical stability term

    // ------------------------------------------------------------
    // Problem size (minimal non‑trivial)
    // ------------------------------------------------------------
    int dim   = 1;              // dimensionality of the parameter vector
    int steps = 1;              // number of Adadelta updates to perform

    // ------------------------------------------------------------
    // Allocate all vectors that will be needed (std::vector usage)
    // ------------------------------------------------------------
    std::vector<float> paramVec(dim);      // parameters (weights)
    std::vector<float> gradVec(dim);       // gradients (generated each step)
    std::vector<float> avgSqGrad(dim, 0.0f); // E[g^2] accumulator
    std::vector<float> avgSqUpd(dim, 0.0f);  // E[Δx^2] accumulator
    std::vector<float> deltaParam(dim, 0.0f); // Δx for current step

    // ------------------------------------------------------------
    // Initialise parameters with random numbers in [-1, 1]
    // ------------------------------------------------------------
    for (int i = 0; i < dim; ++i) {
        float rnd = (float)std::rand() / (float)RAND_MAX; // [0,1]
        paramVec[i] = rnd * 2.0f - 1.0f;                    // [-1,1]
    }

    // ------------------------------------------------------------
    // Main optimisation loop (while‑loop – different loop structure)
    // ------------------------------------------------------------
    int iter = 0;
    while (iter < steps) {
        // ---- generate a fresh random gradient vector for this step ----
        for (int j = 0; j < dim; ++j) {
            float rnd = (float)std::rand() / (float)RAND_MAX;
            gradVec[j] = rnd * 2.0f - 1.0f;
        }

        // ---- element‑wise Adadelta update (verbose, reordered math) ----
        for (int k = 0; k < dim; ++k) {
            // 1) update running average of squared gradients
            float gradSq   = gradVec[k] * gradVec[k];
            float termA    = rho * avgSqGrad[k];
            float termB    = (1.0f - rho) * gradSq;
            avgSqGrad[k]   = termA + termB;          // reordered: termA + termB

            // 2) compute RMS of gradients (sqrt of accumulated + eps)
            float rmsGrad  = std::sqrt(avgSqGrad[k] + eps);

            // 3) compute RMS of previous parameter updates
            float rmsUpdPrev = std::sqrt(avgSqUpd[k] + eps);

            // 4) compute the actual update Δx
            //    reordered: multiply first, then divide
            float numer    = rmsUpdPrev * gradVec[k];
            deltaParam[k]  = - numer / rmsGrad;

            // 5) update running average of squared updates
            float deltaSq  = deltaParam[k] * deltaParam[k];
            float updA     = rho * avgSqUpd[k];
            float updB     = (1.0f - rho) * deltaSq;
            avgSqUpd[k]    = updA + updB;            // reordered: updA + updB

            // 6) apply the update to the parameter
            paramVec[k]    = paramVec[k] + deltaParam[k];
        }

        ++iter;
    }

    // ------------------------------------------------------------
    // Output the final parameters
    // ------------------------------------------------------------
    std::cout << "Final parameters after " << steps << " Adadelta steps:\n";
    for (int i = 0; i < dim; ++i) {
        std::cout << "p[" << i << "] = " << paramVec[i] << "\n";
    }

    return 0;
}
