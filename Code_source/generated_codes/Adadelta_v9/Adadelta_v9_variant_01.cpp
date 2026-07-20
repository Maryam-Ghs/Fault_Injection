// LLM input variant 1: minimal-boundary
// Adadelta optimizer – version #9
#include <iostream>
#include <cmath>

class AdaDelta {
public:
    // hyper‑parameters
    float decayRate;
    float eps;

    // problem size
    int dim;

    // state stored on the heap
    float* avgGradSq;   // E[g^2]_{t}
    float* avgUpdSq;    // E[Δ^2]_{t}

    // constructor – allocate and initialise state
    AdaDelta(int d, float rho, float epsilon) {
        dim        = d;
        decayRate  = rho;
        eps        = epsilon;
        avgGradSq  = new float[dim];
        avgUpdSq   = new float[dim];
        int i = 0;
        while (i < dim) {
            avgGradSq[i] = 0.0f;
            avgUpdSq[i]  = 0.0f;
            ++i;
        }
    }

    // destructor – free heap memory
    ~AdaDelta() {
        delete[] avgGradSq;
        delete[] avgUpdSq;
    }

    // one optimisation step
    void step(float* param, const float* grad) {
        int j = 0;
        while (j < dim) {
            // 1) update running average of squared gradients
            float g2      = grad[j] * grad[j];
            float newAvgG = decayRate * avgGradSq[j] + (1.0f - decayRate) * g2;

            // 2) compute adaptive learning rate
            float rmsUpd  = std::sqrt(avgUpdSq[j] + eps);
            float rmsGrad = std::sqrt(newAvgG      + eps);
            float adapt   = rmsUpd / rmsGrad;

            // 3) compute parameter update
            float delta   = -adapt * grad[j];

            // 4) apply update
            param[j] += delta;

            // 5) update running average of squared updates
            float d2      = delta * delta;
            float newAvgU = decayRate * avgUpdSq[j] + (1.0f - decayRate) * d2;

            // 6) store the new averages
            avgGradSq[j] = newAvgG;
            avgUpdSq[j]  = newAvgU;

            ++j;
        }
    }
};

int main() {
    // deterministic test – minimal size: 1 parameter, 1 optimisation step
    const int size = 1;
    float* theta   = new float[size];
    float* grad    = new float[size];

    // initialise parameter
    int k = 0;
    while (k < size) {
        theta[k] = 1.0f;   // start from 1.0
        ++k;
    }

    // create optimizer instance
    AdaDelta opt(size, 0.95f, 1e-6f);

    // single deterministic gradient vector
    float g0[1] = { 0.1f };

    // step 1
    int i = 0;
    while (i < size) {
        grad[i] = g0[i];
        ++i;
    }
    opt.step(theta, grad);

    // print final parameter
    std::cout << "Final parameter after Adadelta update:\n";
    i = 0;
    while (i < size) {
        std::cout << "theta[" << i << "] = " << theta[i] << "\n";
        ++i;
    }

    delete[] theta;
    delete[] grad;
    return 0;
}
