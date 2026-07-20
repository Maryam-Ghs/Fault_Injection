// Adadelta optimizer – version #9
/* LLM input variant 9: medium-deterministic-random */
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
    // deterministic test – 12 parameters, 4 optimisation steps
    const int size = 12;
    float* theta   = new float[size];
    float* grad    = new float[size];

    // initialise parameters
    int k = 0;
    while (k < size) {
        theta[k] = 1.0f;   // start from 1.0 for every dimension
        ++k;
    }

    // create optimizer instance
    AdaDelta opt(size, 0.95f, 1e-6f);

    // four deterministic gradient vectors (pseudo‑random looking)
    float g0[12] = { 0.12f, -0.23f, 0.07f, -0.14f, 0.19f, -0.05f, 0.11f, -0.09f, 0.03f, -0.17f, 0.08f, -0.02f };
    float g1[12] = { -0.06f, 0.15f, -0.12f, 0.04f, -0.08f, 0.22f, -0.03f, 0.09f, -0.11f, 0.05f, -0.14f, 0.13f };
    float g2[12] = { 0.21f, -0.18f, 0.13f, -0.25f, 0.07f, -0.09f, 0.16f, -0.04f, 0.2f, -0.12f, 0.03f, -0.07f };
    float g3[12] = { -0.11f, 0.09f, -0.05f, 0.17f, -0.13f, 0.06f, -0.2f, 0.14f, -0.08f, 0.22f, -0.01f, 0.05f };

    // step 1
    int i = 0;
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

    // step 4
    i = 0;
    while (i < size) {
        grad[i] = g3[i];
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
