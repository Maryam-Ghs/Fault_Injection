/* LLM input variant 9: medium-deterministic-random */
#include <iostream>
#include <cmath>
#include <cstdint>

int main() {
    // problem size (medium)
    const int dim = 12;

    // hyper‑parameters (float only)
    const float rho = 0.95f;
    const float eps = 1e-6f;

    // stack‑allocated vectors
    float theta[12];
    float grad[12];
    float accGrad[12];
    float accUpd[12];

    // deterministic pseudo‑random generator (LCG)
    uint32_t seed = 0xDEADBEEF;
    auto next_float = [&seed]() -> float {
        seed = seed * 1664525u + 1013904223u;
        // produce a float in [-1, 1]
        return (static_cast<float>(seed % 20001u) / 10000.0f) - 1.0f;
    };

    // initialise parameters, gradients and accumulators
    for (int i = 0; i < dim; ++i) {
        theta[i]   = next_float(); // [-1,1]
        grad[i]    = next_float(); // [-1,1]
        accGrad[i] = 0.0f;
        accUpd[i]  = 0.0f;
    }

    // number of AdaDelta steps (medium)
    int step = 0;
    const int maxStep = 15;

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
