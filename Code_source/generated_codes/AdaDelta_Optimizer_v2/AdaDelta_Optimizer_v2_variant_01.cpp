/* LLM input variant 1: minimal-boundary */
#include <iostream>
#include <cstdlib>
#include <cmath>

int main() {
    // problem size (minimal nontrivial)
    int dim = 1;

    // hyper‑parameters (float only)
    float rho = 0.95f;
    float eps = 1e-6f;

    // stack‑allocated vectors (size matches dim)
    float theta[1];
    float grad[1];
    float accGrad[1];
    float accUpd[1];

    // initialise parameters, gradients and accumulators
    for (int i = 0; i < dim; ++i) {
        theta[i]   = (static_cast<float>(std::rand()) / RAND_MAX) * 2.0f - 1.0f; // [-1,1]
        grad[i]    = (static_cast<float>(std::rand()) / RAND_MAX) * 2.0f - 1.0f; // [-1,1]
        accGrad[i] = 0.0f;
        accUpd[i]  = 0.0f;
    }

    // number of AdaDelta steps (minimal nontrivial)
    int step = 0;
    int maxStep = 1;

    while (step < maxStep) {
        int idx = 0;
        while (idx < dim) {
            // 1) update accumulated squared gradients (reordered arithmetic)
            float g2 = grad[idx] * grad[idx];
            accGrad[idx] = rho * accGrad[idx] + (1.0f - rho) * g2;

            // 2) compute adaptive update
            float num = accUpd[idx] + eps;          // numerator inside sqrt
            float den = accGrad[idx] + eps;         // denominator inside sqrt
            float rmsUpd = sqrtf(num);
            float rmsGrad = sqrtf(den);
            float delta = (rmsUpd / rmsGrad) * grad[idx]; // AdaDelta update

            // 3) update accumulated squared updates (reordered)
            float d2 = delta * delta;
            accUpd[idx] = rho * accUpd[idx] + (1.0f - rho) * d2;

            // 4) apply update to parameters
            theta[idx] = theta[idx] - delta;

            ++idx;
        }
        ++step;
    }

    // print final parameters
    std::cout << "Final parameters after AdaDelta:\n";
    for (int i = 0; i < dim; ++i) {
        std::cout << "theta[" << i << "] = " << theta[i] << '\n';
    }

    return 0;
}
