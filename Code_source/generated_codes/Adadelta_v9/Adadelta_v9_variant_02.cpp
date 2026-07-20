// LLM input variant 2: small-diverse
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
    // deterministic test – 6 parameters, 3 optimisation steps
    const int size = 6;
    float* theta   = new float[size];
    float* grad    = new float[size];

    // initialise parameters with varied starting values
    int k = 0;
    while (k < size) {
        theta[k] = 0.5f + 0.3f * static_cast<float>(k);   // 0.5, 0.8, 1.1, 1.4, 1.7, 2.0
        ++k;
    }

    // create optimizer instance with slightly different hyper‑parameters
    AdaDelta opt(size, 0.90f, 1e-5f);

    // three deterministic gradient vectors with diverse values
    float g0[6] = { 0.12f, -0.25f, 0.07f, -0.15f, 0.22f, -0.05f };
    float g1[6] = { -0.08f, 0.14f, -0.09f, 0.06f, -0.12f, 0.03f };
    float g2[6] = { 0.18f, -0.11f, 0.13f, -0.19f, 0.04f, -0.07f };

    // step 1
    i = 0;
    while (i < size) {
        grad[i] = g0[i];
        ++i;
    }
    opt.step(theta, grad);

    // step 2
    i = 0;
    while (i < size) {
        grad[i] = g1[i];
        ++i;
    }
    opt.step(theta, grad);

    // step 3
    i = 0;
    while (i < size) {
        grad[i] = g2[i];
        ++i;
    }
    opt.step(theta, grad);

    // print final parameters
    std::cout << "Final parameters after Adadelta updates:\n";
    i = 0;
    while (i < size) {
        std::cout << "theta[" << i << "] = " << theta[i] << "\n";
        ++i;
    }

    delete[] theta;
    delete[] grad;
    return 0;
}
