/* LLM input variant 4: signed-extremes */
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
    // deterministic test – 7 parameters, 3 optimisation steps
    const int size = 7;
    float* theta   = new float[size];
    float* grad    = new float[size];

    // initialise parameters with a mix of negative, zero, and positive values
    int k = 0;
    while (k < size) {
        if (k == 0) theta[k] = -2.0f;
        else if (k == 1) theta[k] = 0.0f;
        else if (k == 2) theta[k] = 1.5f;
        else if (k == 3) theta[k] = -1.0f;
        else if (k == 4) theta[k] = 2.0f;
        else if (k == 5) theta[k] = 0.0f;
        else            theta[k] = -0.5f;
        ++k;
    }

    // create optimizer instance
    AdaDelta opt(size, 0.95f, 1e-6f);

    // three deterministic gradient vectors with mixed signs and zeros
    float g0[7] = {  0.3f,  0.0f, -0.4f,  0.2f, -0.1f,  0.0f,  0.5f };
    float g1[7] = { -0.2f,  0.1f,  0.0f, -0.3f,  0.4f, -0.5f,  0.0f };
    float g2[7] = {  0.0f, -0.3f,  0.2f,  0.0f, -0.2f,  0.3f, -0.4f };

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
