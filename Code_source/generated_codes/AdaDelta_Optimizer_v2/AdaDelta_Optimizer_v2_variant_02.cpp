#include <iostream>
#include <cmath>

/* LLM input variant 2: small-diverse */

int main() {
    // problem size
    int dim = 5;

    // hyper‑parameters (float only)
    float rho = 0.95f;
    float eps = 1e-6f;

    // stack‑allocated vectors
    float theta[5];
    float grad[5];
    float accGrad[5];
    float accUpd[5];

    // deterministic diverse initialization
    float initTheta[5] = { 0.5f, -0.3f, 0.0f, 0.9f, -0.8f };
    float initGrad[5]  = { 0.1f, -0.2f, 0.3f, -0.4f, 0.5f };
    for (int i = 0; i < dim; ++i) {
        theta[i]   = initTheta[i];
        grad[i]    = initGrad[i];
        accGrad[i] = 0.0f;
        accUpd[i]  = 0.0f;
    }

    // number of AdaDelta steps
    int step = 0;
    int maxStep = 10;

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
