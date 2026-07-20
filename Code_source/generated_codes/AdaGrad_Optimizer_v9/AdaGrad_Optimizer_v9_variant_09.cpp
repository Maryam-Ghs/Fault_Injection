#include <iostream>
#include <cmath>

/* LLM input variant 9: medium-deterministic-random */

// ------------------------------------------------------------
// AdaGrad Optimizer - Version #9
// ------------------------------------------------------------

// Allocate a float array on the heap
float* createArray(int size) {
    float* arr = new float[size];
    for (int i = 0; i < size; ++i) {
        arr[i] = 0.0f;
    }
    return arr;
}

// Release a heap array
void destroyArray(float* arr) {
    delete[] arr;
}

// Compute the squared L2 loss: 0.5 * sum (param - target)^2
float computeLoss(const float* param, const float* target, int dim) {
    float loss = 0.0f;
    for (int i = 0; i < dim; ++i) {
        float diff = param[i] - target[i];
        loss += diff * diff;
    }
    return 0.5f * loss;
}

// Compute gradient of the quadratic loss: grad = param - target
void computeGradient(const float* param, const float* target,
                     float* grad, int dim) {
    for (int i = 0; i < dim; ++i) {
        grad[i] = param[i] - target[i];
    }
}

// Perform a single AdaGrad update step
void adagradStep(float* param, const float* grad,
                 float* accum, float stepSize, float eps, int dim) {
    // Update accumulator first (sum of squares of gradients)
    for (int i = 0; i < dim; ++i) {
        accum[i] += grad[i] * grad[i];
    }

    // Now adjust learning rates and update parameters
    for (int i = 0; i < dim; ++i) {
        // Compute adjusted step: stepSize / (sqrt(accum) + eps)
        float adj = stepSize / (std::sqrt(accum[i]) + eps);
        // Parameter update: param = param - adj * grad
        param[i] -= adj * grad[i];
    }
}

// AdaGrad optimizer encapsulated in a class
class AdaGradOpt {
public:
    AdaGradOpt(float lr, float epsilon, int iters)
        : learningRate(lr), eps(epsilon), maxIter(iters) {}

    // Run optimizer on a given problem
    void run(float* initParam, const float* target, int dim) {
        // Allocate gradient and accumulator on the heap
        float* gradVec = createArray(dim);
        float* accumVec = createArray(dim);

        // Main optimization loop
        for (int step = 0; step < maxIter; ++step) {
            // 1) Compute gradient based on current parameters
            computeGradient(initParam, target, gradVec, dim);

            // 2) Perform AdaGrad parameter update
            adagradStep(initParam, gradVec, accumVec,
                        learningRate, eps, dim);

            // 3) (Optional) Print intermediate loss for edge‑case monitoring
            if ((step + 1) % 10 == 0) {
                float curLoss = computeLoss(initParam, target, dim);
                std::cout << "Iter " << (step + 1)
                          << " | Loss: " << curLoss << std::endl;
            }
        }

        // Clean up heap memory
        destroyArray(gradVec);
        destroyArray(accumVec);
    }

private:
    float learningRate; // Base step size
    float eps;          // Small constant to avoid division by zero
    int   maxIter;      // Number of iterations
};

// Simple deterministic pseudo‑random generator (LCG)
static unsigned int lcg_rand(unsigned int& state) {
    state = state * 1103515245u + 12345u;
    return (state >> 16) & 0x7FFFu;
}

// ------------------------------------------------------------
// Main: generate a medium‑deterministic‑random synthetic problem
// ------------------------------------------------------------
int main() {
    // Problem dimension (medium size)
    const int dim = 1000;

    // Allocate parameter vector and target vector on the heap
    float* paramVec = createArray(dim);
    float* targetVec = createArray(dim);

    // Deterministic pseudo‑random initialization for parameters
    unsigned int seed = 987654321u; // fixed seed
    for (int i = 0; i < dim; ++i) {
        unsigned int r = lcg_rand(seed);
        // Map to range [-5000.0, 5000.0]
        float val = (static_cast<float>(r) / 32767.0f) * 10000.0f - 5000.0f;
        paramVec[i] = val;
    }

    // Deterministic pseudo‑random initialization for targets
    unsigned int tseed = 123456789u; // another fixed seed
    for (int i = 0; i < dim; ++i) {
        unsigned int r = lcg_rand(tseed);
        // For even indices use a small random value, odd indices zero
        if (i % 2 == 0) {
            float val = (static_cast<float>(r) / 32767.0f) * 0.01f; // [0,0.01]
            targetVec[i] = val;
        } else {
            targetVec[i] = 0.0f;
        }
    }

    // AdaGrad hyper‑parameters tuned for this synthetic data
    float baseLR   = 0.5f;    // moderate base learning rate
    float epsilon  = 1e-6f; // small epsilon to avoid div‑zero
    int   maxSteps = 120;   // a bit more iterations

    // Create optimizer instance and run
    AdaGradOpt optimizer(baseLR, epsilon, maxSteps);
    optimizer.run(paramVec, targetVec, dim);

    // Final loss
    float finalLoss = computeLoss(paramVec, targetVec, dim);
    std::cout << "Final loss: " << finalLoss << std::endl;

    // Print a few final parameters to verify convergence
    std::cout << "Sample final parameters:" << std::endl;
    for (int i = 0; i < 5; ++i) {
        std::cout << "param[" << i << "] = " << paramVec[i] << std::endl;
    }

    // Clean up
    destroyArray(paramVec);
    destroyArray(targetVec);

    return 0;
}
