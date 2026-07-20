#include <iostream>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <ctime>

/* LLM input variant 4: signed-extremes */

int main() {
    // ------------------------------------------------------------
    // Seed the pseudo‑random generator (kept for reproducibility)
    // ------------------------------------------------------------
    std::srand((int)std::time(0));

    // ------------------------------------------------------------
    // Hyper‑parameters for Adadelta (float only)
    // ------------------------------------------------------------
    float rho = 0.95f;          // decay factor
    float eps = 1e-6f;          // numerical stability term

    // ------------------------------------------------------------
    // Problem size (medium deterministic arrays)
    // ------------------------------------------------------------
    int dim   = 8;              // dimensionality of the parameter vector
    int steps = 15;             // number of Adadelta updates to perform

    // ------------------------------------------------------------
    // Allocate all vectors that will be needed (std::vector usage)
    // ------------------------------------------------------------
    std::vector<float> paramVec(dim);      // parameters (weights)
    std::vector<float> gradVec(dim);       // gradients (generated each step)
    std::vector<float> avgSqGrad(dim, 0.0f); // E[g^2] accumulator
    std::vector<float> avgSqUpd(dim, 0.0f);  // E[Δx^2] accumulator
    std::vector<float> deltaParam(dim, 0.0f); // Δx for current step

    // ------------------------------------------------------------
    // Initialise parameters with a deterministic mix of -1, 0, and 1
    // ------------------------------------------------------------
    for (int i = 0; i < dim; ++i) {
        if (i % 3 == 0) {
            paramVec[i] = -1.0f;
        } else if (i % 3 == 1) {
            paramVec[i] = 0.0f;
        } else {
            paramVec[i] = 1.0f;
        }
    }

    // ------------------------------------------------------------
    // Main optimisation loop (while‑loop – different loop structure)
    // ------------------------------------------------------------
    int iter = 0;
    while (iter < steps) {
        // ---- generate a deterministic gradient vector for this step ----
        for (int j = 0; j < dim; ++j) {
            int pattern = (iter + j) % 4;
            if (pattern == 0) {
                gradVec[j] = -0.5f;
            } else if (pattern == 1) {
                gradVec[j] = 0.0f;
            } else if (pattern == 2) {
                gradVec[j] = 0.5f;
            } else {
                gradVec[j] = 1.0f;
            }
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
