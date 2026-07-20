#include <iostream>
#include <cmath>

/* LLM input variant 7: reverse-adversarial */
int main() {
    /* -------------------------------------------------------------
       Deterministic toy data (reverse‑ordered, adversarial):
       x values   : 5.0 , 4.0 , 3.0 , 2.0 , 1.0
       target y   : -10.0 , -8.0 , -6.0 , -4.0 , -2.0   (y = -2 * x, reversed order)
       ------------------------------------------------------------- */

    const int dataSize = 5;                     // number of samples
    int dataCount = dataSize;                   // mutable copy

    // Allocate input and output vectors on the heap
    float* vecX = new float[dataCount];
    float* vecY = new float[dataCount];

    // Fill vectors with deterministic adversarial values (reverse order)
    vecX[0] = 5.0f; vecY[0] = -10.0f;
    vecX[1] = 4.0f; vecY[1] = -8.0f;
    vecX[2] = 3.0f; vecY[2] = -6.0f;
    vecX[3] = 2.0f; vecY[3] = -4.0f;
    vecX[4] = 1.0f; vecY[4] = -2.0f;

    /* -------------------------------------------------------------
       Model parameters (weight and bias) and their Adagrad accumulators
       ------------------------------------------------------------- */
    float weight   = 0.0f;      // initial weight
    float bias     = 0.0f;      // initial bias
    float accW     = 0.0f;      // accumulated squared gradient for weight
    float accB     = 0.0f;      // accumulated squared gradient for bias

    // Hyper‑parameters
    float learnRate = 0.5f;     // base learning rate
    float epsilon   = 1e-8f;    // small constant to avoid division by zero

    // Training control
    int epochIdx = 0;           // epoch counter
    int maxEpoch = 5;           // run a few epochs for demonstration

    // -------------------------------------------------------------
    // Training loop (verbose, step‑by‑step)
    // -------------------------------------------------------------
    while (epochIdx < maxEpoch) {
        // ---- 1. Zero‑out gradient accumulators for this epoch ----
        float gradW = 0.0f;
        float gradB = 0.0f;

        // ---- 2. Compute gradients over the whole data set ----
        int i = dataCount;                 // iterate backwards to change loop shape
        while (i--) {
            // ---- a) forward pass: prediction ----
            float pred = weight * vecX[i] + bias;

            // ---- b) error term (prediction - target) ----
            float err = pred - vecY[i];

            // ---- c) contribution to gradient (MSE derivative) ----
            //    dLoss/dWeight = (2/N) * err * x_i
            //    dLoss/dBias   = (2/N) * err
            float twoOverN = 2.0f / static_cast<float>(dataCount);
            float termW = twoOverN * err * vecX[i];
            float termB = twoOverN * err;

            // ---- d) accumulate gradient contributions ----
            gradW += termW;
            gradB += termB;
        }

        // ---- 3. Square the gradients ----
        float gradWSq = gradW * gradW;
        float gradBSq = gradB * gradB;

        // ---- 4. Update the Adagrad accumulators ----
        accW += gradWSq;
        accB += gradBSq;

        // ---- 5. Compute adjusted learning rates (per‑parameter) ----
        float adjLR_W = learnRate / std::sqrt(accW + epsilon);
        float adjLR_B = learnRate / std::sqrt(accB + epsilon);

        // ---- 6. Compute parameter updates (delta) ----
        float deltaW = adjLR_W * gradW;
        float deltaB = adjLR_B * gradB;

        // ---- 7. Apply updates ----
        weight -= deltaW;
        bias   -= deltaB;

        // ---- 8. Print epoch summary (verbose) ----
        std::cout << "Epoch " << epochIdx + 1 << ":\n";
        std::cout << "  gradW = " << gradW << " , gradB = " << gradB << "\n";
        std::cout << "  accW  = " << accW << " , accB  = " << accB << "\n";
        std::cout << "  adjLR_W = " << adjLR_W << " , adjLR_B = " << adjLR_B << "\n";
        std::cout << "  deltaW = " << deltaW << " , deltaB = " << deltaB << "\n";
        std::cout << "  weight = " << weight << " , bias = " << bias << "\n\n";

        // ---- 9. Advance epoch counter ----
        epochIdx = epochIdx + 1;
    }

    // -------------------------------------------------------------
    // Final model parameters
    // -------------------------------------------------------------
    std::cout << "Training completed.\n";
    std::cout << "Final weight = " << weight << "\n";
    std::cout << "Final bias   = " << bias << "\n";

    // Clean up heap memory
    delete[] vecX;
    delete[] vecY;

    return 0;
}
