#include <iostream>
#include <cmath>

/* LLM input variant 6: ordered-structured */

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

    // initialise parameters, gradients and accumulators with ordered values
    // theta: linearly spaced from -1 to 1
    // grad: linearly spaced from 1 to -1 (reverse order)
    for (int i = 0; i < dim; ++i) {
        theta[i]   = -1.0f + (2.0f * i) / (dim - 1);   // -1, -0.5, 0, 0.5, 1
        grad[i]    =  1.0f - (2.0f * i) / (dim - 1);   //  1, 0.5, 0, -0.5, -1
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
            float den = accGrad[idx] + eps;          // denominator inside sqrt
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
