/* LLM input variant 8: sparse-skewed */
#include <iostream>
#include <cmath>

int main() {
    // problem size (expanded to illustrate sparsity)
    const int dim = 10;

    // hyper‑parameters (float only)
    const float rho = 0.95f;
    const float eps = 1e-6f;

    // stack‑allocated vectors
    float theta[dim];
    float grad[dim];
    float accGrad[dim];
    float accUpd[dim];

    // initialise parameters with a sparse, clustered pattern
    for (int i = 0; i < dim; ++i) {
        // Majority are zero
        theta[i] = 0.0f;
        grad[i]  = 0.0f;
        accGrad[i] = 0.0f;
        accUpd[i]  = 0.0f;
    }
    // Clustered non‑zero entries
    theta[0] =  0.8f;
    theta[1] = -0.6f;
    grad[0]  =  0.03f;
    grad[1]  = -0.04f;
    // A single outlier far from the cluster
    theta[8] =  0.0f;
    grad[8]  =  0.001f;

    // number of AdaDelta steps
    int step = 0;
    const int maxStep = 10;

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
