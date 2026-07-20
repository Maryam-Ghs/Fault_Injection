#include <iostream>
#include <cmath>

/* LLM input variant 10: large-safe-stress */

int main() {
    // problem size
    const int dim = 500;

    // hyper‑parameters (float only)
    const float rho = 0.95f;
    const float eps = 1e-6f;

    // heap‑allocated vectors to handle larger size safely
    float* theta   = new float[dim];
    float* grad    = new float[dim];
    float* accGrad = new float[dim];
    float* accUpd  = new float[dim];

    // deterministic initialization within [-1,1]
    for (int i = 0; i < dim; ++i) {
        theta[i]   = static_cast<float>((i % 200) - 100) / 100.0f;          // [-1,1]
        grad[i]    = static_cast<float>(((i * 3) % 200) - 100) / 100.0f;   // [-1,1]
        accGrad[i] = 0.0f;
        accUpd[i]  = 0.0f;
    }

    // number of AdaDelta steps
    int step = 0;
    const int maxStep = 100;

    while (step < maxStep) {
        for (int idx = 0; idx < dim; ++idx) {
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
        }
        ++step;
    }

    // print final parameters
    std::cout << "Final parameters after AdaDelta:\n";
    for (int i = 0; i < dim; ++i) {
        std::cout << "theta[" << i << "] = " << theta[i] << '\n';
    }

    delete[] theta;
    delete[] grad;
    delete[] accGrad;
    delete[] accUpd;

    return 0;
}
