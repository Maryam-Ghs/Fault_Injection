/* LLM input variant 2: small-diverse */
// Adagrad optimizer implementation – version #5
// ------------------------------------------------
// Simple linear regression (y ≈ w * x + b) trained with Adagrad.
// Only float and int are used. No input from cin; data are generated internally.

#include <iostream>
#include <vector>
#include <cmath>   // for std::fabs (allowed, works with float)

class AdaGradOptimizer {
public:
    // Constructor: learning rate and epsilon for numerical stability
    AdaGradOptimizer(float lr, float eps) : learningRate(lr), epsilon(eps) {}

    // Initialize internal accumulation vectors to match parameter size
    void init(const std::vector<float>& initParams) {
        accum.assign(initParams.size(), 0.0f);
    }

    // Perform a single optimization step given gradients
    void step(std::vector<float>& params, const std::vector<float>& grads) {
        // Loop over each parameter (different loop style)
        int idx = 0;
        while (idx < static_cast<int>(params.size())) {
            // Accumulate squared gradients
            float gradSq = grads[idx] * grads[idx];
            accum[idx] = accum[idx] + gradSq;

            // Compute adjusted learning rate for this parameter
            float adjLR = learningRate / (std::sqrt(accum[idx]) + epsilon);

            // Update parameter (expanded multi‑step)
            float delta = -adjLR * grads[idx];
            float newVal = params[idx] + delta;
            params[idx] = newVal;

            ++idx;
        }
    }

private:
    float learningRate;
    float epsilon;
    std::vector<float> accum;   // accumulated squared gradients
};

int main() {
    // ------------------- Generate deterministic synthetic data -------------------
    const int sampleCount = 12;                // small diverse dataset
    std::vector<float> dataX = {
        -5.0f, -2.5f, -1.0f, 0.0f, 0.5f, 1.2f,
        3.3f, 4.7f, 5.5f, 7.8f, 9.0f, 9.9f
    };
    std::vector<float> dataY = {
        -8.2f, -4.60f, -2.1f, -0.9f, -0.05f, 1.05f,
        4.08f, 6.37f, 7.42f, 10.98f, 12.61f, 14.09f
    };

    // ------------------- Model parameters -------------------
    // params[0] = weight (w), params[1] = bias (b)
    std::vector<float> params(2);
    params[0] = 0.0f;   // initial weight
    params[1] = 0.0f;   // initial bias

    // ------------------- Optimizer setup -------------------
    AdaGradOptimizer optimizer(0.3f, 1e-6f);
    optimizer.init(params);

    // ------------------- Training loop -------------------
    const int epochs = 60;
    for (int ep = 0; ep < epochs; ++ep) {
        // Accumulate gradients for the whole batch (simple SGD batch)
        std::vector<float> grad(2, 0.0f);   // grad[0] = dL/dw, grad[1] = dL/db
        float totalLoss = 0.0f;

        // Different loop style: for‑each with manual index
        for (int i = 0; i < sampleCount; ++i) {
            // ----- Forward pass -----
            float pred = params[0] * dataX[i] + params[1];

            // ----- Compute error -----
            float err = pred - dataY[i];

            // ----- Accumulate loss (expanded) -----
            float sqErr = err * err;
            totalLoss = totalLoss + 0.5f * sqErr;

            // ----- Backward pass (gradients) -----
            // dL/dw = err * x
            // dL/db = err
            float gradW_i = err * dataX[i];
            float gradB_i = err;

            grad[0] = grad[0] + gradW_i;
            grad[1] = grad[1] + gradB_i;
        }

        // Average gradients over the batch
        float invN = 1.0f / static_cast<float>(sampleCount);
        grad[0] = grad[0] * invN;
        grad[1] = grad[1] * invN;

        // ------------------- Parameter update -------------------
        optimizer.step(params, grad);

        // Optional: print loss every 15 epochs
        if ((ep + 1) % 15 == 0) {
            std::cout << "Epoch " << (ep + 1)
                      << " | Loss: " << totalLoss << std::endl;
        }
    }

    // ------------------- Final results -------------------
    std::cout << "\nTraining completed.\n";
    std::cout << "Estimated weight (w): " << params[0] << std::endl;
    std::cout << "Estimated bias   (b): " << params[1] << std::endl;

    // Compute final loss for verification
    float finalLoss = 0.0f;
    for (int i = 0; i < sampleCount; ++i) {
        float pred = params[0] * dataX[i] + params[1];
        float err = pred - dataY[i];
        finalLoss = finalLoss + 0.5f * err * err;
    }
    std::cout << "Final loss: " << finalLoss << std::endl;

    return 0;
}
