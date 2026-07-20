#include <iostream>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <ctime>

/* LLM input variant 8: sparse-skewed */

int main() {
    // ------------------------------------------------------------
    // Seed the pseudo‑random generator (no unsigned used explicitly)
    // ------------------------------------------------------------
    std::srand((int)std::time(0));

    // ------------------------------------------------------------
    // Hyper‑parameters for Adadelta (float only)
    // ------------------------------------------------------------
    float rho = 0.95f;          // decay factor
    float eps = 1e-6f;          // numerical stability term

    // ------------------------------------------------------------
    // Problem size (sparse, larger dimensionality)
    // ------------------------------------------------------------
    int dim   = 20;              // dimensionality of the parameter vector
    int steps = 15;             // number of Adadelta updates to perform

    // ------------------------------------------------------------
    // Allocate all vectors that will be needed (std::vector usage)
    // ------------------------------------------------------------
    std::vector<float> paramVec(dim, 0.0f);      // parameters (weights)
    std::vector<float> gradVec(dim, 0.0f);       // gradients (generated each step)
    std::vector<float> avgSqGrad(dim, 0.0f); // E[g^2] accumulator
    std::vector<float> avgSqUpd(dim, 0.0f);  // E[Δx^2] accumulator
    std::vector<float> deltaParam(dim, 0.0f); // Δx for current step

    // ------------------------------------------------------------
    // Initialise parameters with a sparse pattern
    // ------------------------------------------------------------
    paramVec[5]  =  0.7f;   // isolated non‑zero entry
    paramVec[15] = -0.4f;   // another isolated non‑zero entry

    // ------------------------------------------------------------
    // Main optimisation loop (while‑loop – different loop structure)
    // ------------------------------------------------------------
    int iter = 0;
    while (iter < steps) {
        // ---- generate a sparse gradient vector for this step ----
        // Reset to zero
        std::fill(gradVec.begin(), gradVec.end(), 0.0f);
        // Define a small cluster of non‑zero gradients
        int baseIdx = (iter * 3) % dim;
        for (int offset = 0; offset < 3; ++offset) {
            int idx = (baseIdx + offset) % dim;
            float rnd = (float)std::rand() / (float)RAND_MAX; // [0,1]
            gradVec[idx] = rnd * 2.0f - 1.0f;                    // [-1,1]
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
