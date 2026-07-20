#include <iostream>
#include <cmath>

/* LLM input variant 5: duplicate-heavy */

// Version 6 - AmsGrad implementation (heap‑based, class‑oriented)

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
    // Duplicate‑heavy predefined problem (size = 8)
    int size = 8;

    // Allocate parameter and gradient buffers on the heap
    float* paramVec = new float[size];
    float* gradVec  = new float[size];

    // Initialise parameters with repeated patterns
    paramVec[0] = 1.0f;  paramVec[1] = 1.0f;
    paramVec[2] = -0.5f; paramVec[3] = -0.5f;
    paramVec[4] = 2.0f;  paramVec[5] = 2.0f;
    paramVec[6] = 0.0f;  paramVec[7] = 0.0f;

    // Initialise gradients with duplicated values
    gradVec[0] = 0.1f;  gradVec[1] = 0.1f;
    gradVec[2] = -0.2f; gradVec[3] = -0.2f;
    gradVec[4] = 0.05f; gradVec[5] = 0.05f;
    gradVec[6] = 0.0f;  gradVec[7] = 0.0f;

    // Create AmsGrad optimizer instance
    AmsGrad optimizer(0.05f, 0.9f, 0.999f, 1e-8f);

    // Run the optimizer (updates printed inside the class)
    optimizer.step(paramVec, gradVec, size);

    // Clean up main buffers
    delete[] paramVec;
    delete[] gradVec;

    return 0;
}
