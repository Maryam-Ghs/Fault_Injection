#include <iostream>
#include <vector>
#include <cmath>

/* LLM input variant 2: small-diverse */

int main() {
    // ------------------------------------------------------------
    // Hyper‑parameters for Adadelta (float only)
    // ------------------------------------------------------------
    float rho = 0.95f;          // decay factor
    float eps = 1e-6f;          // numerical stability term

    // ------------------------------------------------------------
    // Problem size (small diverse arrays)
    // ------------------------------------------------------------
    int dim = 5;                // dimensionality of the parameter vector
    int steps = 7;              // number of Adadelta updates to perform

    // ------------------------------------------------------------
    // Fixed deterministic initial parameters
    // ------------------------------------------------------------
    std::vector<float> paramVec = {0.5f, -0.3f, 0.0f, 0.9f, -0.8f};

    // ------------------------------------------------------------
    // Allocate vectors for gradients and accumulators
    // ------------------------------------------------------------
    std::vector<float> gradVec(dim);
    std::vector<float> avgSqGrad(dim, 0.0f); // E[g^2] accumulator
    std::vector<float> avgSqUpd(dim, 0.0f);  // E[Δx^2] accumulator
    std::vector<float> deltaParam(dim, 0.0f); // Δx for current step

    // ------------------------------------------------------------
    // Predefined gradient sequence (small diverse values)
    // ------------------------------------------------------------
    std::vector<std::vector<float>> gradSeq = {
        { 0.1f, -0.2f,  0.05f,  0.3f, -0.1f},
        {-0.4f,  0.25f, -0.15f, -0.05f, 0.2f},
        { 0.3f,  0.1f,  -0.2f,  0.4f, -0.3f},
        {-0.1f, -0.3f,  0.2f, -0.2f,  0.1f},
        { 0.2f,  0.15f, -0.05f, 0.05f, -0.25f},
        {-0.05f, 0.4f,  0.1f, -0.35f, 0.05f},
        { 0.0f, -0.1f,  0.3f,  0.2f, -0.2f}
    };

    // ------------------------------------------------------------
    // Main optimisation loop (while‑loop – different loop structure)
    // ------------------------------------------------------------
    int iter = 0;
    while (iter < steps) {
        // ---- use the predefined gradient vector for this step ----
        for (int j = 0; j < dim; ++j) {
            gradVec[j] = gradSeq[iter][j];
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
