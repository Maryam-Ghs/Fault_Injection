// LLM input variant 10: large-safe-stress
// Adagrad optimizer implementation – version #5
// ------------------------------------------------
// Simple linear regression (y ≈ w * x + b) trained with Adagrad.
// Only float and int are used. No input from cin; data are generated internally.

#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
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
    // Seed random generator with a fixed value for repeatability
    std::srand(42u);

    // ------------------- Generate synthetic data -------------------
    const int sampleCount = 5000;                // larger but safe size
    std::vector<float> dataX(sampleCount);
    std::vector<float> dataY(sampleCount);

    for (int i = 0; i < sampleCount; ++i) {
        // x in [0, 10)
        float xv = static_cast<float>(std::rand()) / RAND_MAX * 10.0f;
        // y = 2 * x + 3 + noise
        float noise = (static_cast<float>(std::rand()) / RAND_MAX - 0.5f) * 0.5f;
        float yv = 2.0f * xv + 3.0f + noise;

        dataX[i] = xv;
        dataY[i] = yv;
    }

    // ------------------- Model parameters -------------------
    // params[0] = weight (w), params[1] = bias (b)
    std::vector<float> params(2);
    params[0] = 0.0f;   // initial weight
    params[1] = 0.0f;   // initial bias

    // ------------------- Optimizer setup -------------------
    AdaGradOptimizer optimizer(0.5f, 1e-7f);
    optimizer.init(params);

    // ------------------- Training loop -------------------
    const int epochs = 200;
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

        // Optional: print loss every 20 epochs
        if ((ep + 1) % 20 == 0) {
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
