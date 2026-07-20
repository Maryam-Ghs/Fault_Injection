#include <iostream>
#include <cmath>

/* LLM input variant 9: medium-deterministic-random */

class AmsGrad {
public:
    // Learning hyper‑parameters (all float, no const)
    float learnRate;
    float betaOne;
    float betaTwo;
    float eps;

    // Constructor (no const usage)
    AmsGrad(float lr, float b1, float b2, float e) {
        learnRate = lr;
        betaOne   = b1;
        betaTwo   = b2;
        eps       = e;
    }

    // Perform one optimization step on a parameter vector
    void step(float* parms, float* grads, int length) {
        // Allocate/initialise first‑, second‑ moment and max‑second‑moment vectors
        float* firstM  = new float[length];
        float* secondM = new float[length];
        float* maxSecM = new float[length];

        // Initialise moments to zero on the first call
        for (int i = 0; i < length; ++i) {
            firstM[i]  = 0.0f;
            secondM[i] = 0.0f;
            maxSecM[i] = 0.0f;
        }

        // Iteration counter (starts at 1)
        int iter = 1;
        // Run a few internal updates to showcase the algorithm
        while (iter <= 5) {
            // Update moments and parameters element‑wise
            int idx = 0;
            while (idx < length) {
                // ---- moment updates ----
                firstM[idx]  = betaOne * firstM[idx] + (1.0f - betaOne) * grads[idx];
                secondM[idx] = betaTwo * secondM[idx] + (1.0f - betaTwo) * grads[idx] * grads[idx];

                // keep the maximum of second moment so far
                if (secondM[idx] > maxSecM[idx])
                    maxSecM[idx] = secondM[idx];

                // ---- parameter update ----
                float denom = sqrtf(maxSecM[idx]) + eps;
                float delta = learnRate * firstM[idx] / denom;
                parms[idx] -= delta;

                ++idx;
            }

            // Print current parameters
            std::cout << "Iter " << iter << ": ";
            for (int j = 0; j < length; ++j) {
                std::cout << parms[j] << (j + 1 == length ? '\n' : ' ');
            }

            ++iter;
        }

        // Clean up moment buffers
        delete[] firstM;
        delete[] secondM;
        delete[] maxSecM;
    }
};

int main() {
    // Medium predefined problem (size = 12)
    int size = 12;

    // Allocate parameter and gradient buffers on the heap
    float* paramVec = new float[size];
    float* gradVec  = new float[size];

    // Initialise parameters with deterministic pseudo‑random values
    float initParams[12] = { 0.8f, -1.2f, 3.4f, -0.7f, 1.5f,
                            -2.3f, 0.0f, 2.1f, -1.8f, 0.9f,
                            -0.4f, 1.2f };
    for (int i = 0; i < size; ++i) {
        paramVec[i] = initParams[i];
    }

    // Initialise gradients with deterministic pseudo‑random values
    float initGrads[12] = { 0.05f, -0.12f, 0.03f, -0.07f, 0.00f,
                           0.09f, -0.04f, 0.02f, -0.08f, 0.06f,
                           -0.01f, 0.00f };
    for (int i = 0; i < size; ++i) {
        gradVec[i] = initGrads[i];
    }

    // Create AmsGrad optimizer instance with different hyper‑parameters
    AmsGrad optimizer(0.03f, 0.85f, 0.95f, 1e-7f);

    // Run the optimizer (updates printed inside the class)
    optimizer.step(paramVec, gradVec, size);

    // Clean up main buffers
    delete[] paramVec;
    delete[] gradVec;

    return 0;
}
