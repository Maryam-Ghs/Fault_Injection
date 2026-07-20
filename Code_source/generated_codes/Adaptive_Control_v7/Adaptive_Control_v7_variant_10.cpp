#include <iostream>
#include <vector>
#include <cmath>

/* LLM input variant 10: large-safe-stress */

int main() {
    // version #7 - adaptive control demonstration
    // deterministic reference trajectory generation (large safe size)

    // simulation length (increased for stress testing)
    const int nSteps = 5000;

    // generate reference trajectory (deterministic sinusoidal array)
    std::vector<float> refSig(nSteps);
    const float twoPi = 2.0f * static_cast<float>(M_PI);
    const float period = 100.0f; // period of the sine wave
    for (int i = 0; i < nSteps; ++i) {
        refSig[i] = std::sin(twoPi * static_cast<float>(i) / period);
    }

    // plant output storage
    std::vector<float> outPlant(nSteps + 1, 0.0f); // y(0)=0

    // adaptive parameters (initial guess)
    float w1 = 0.0f, w2 = 0.0f;

    // adaptation gains (chosen small)
    float g1 = 0.01f, g2 = 0.01f;

    // plant true parameters (unknown to controller)
    float aTrue = 0.6f, bTrue = 0.8f;

    // manual loop unrolling: process two time‑steps per iteration
    int i = 0;
    for (; i + 1 < nSteps; i += 2) {
        // ---- first step (i) ----
        // control law: u = w1*ref + w2*y
        float u0 = w1 * refSig[i] + w2 * outPlant[i];
        // plant update: y_next = a*y + b*u
        outPlant[i + 1] = aTrue * outPlant[i] + bTrue * u0;

        // error e = y - ref
        float e0 = outPlant[i + 1] - refSig[i];
        // parameter update (fused expression)
        w1 = w1 - g1 * e0 * refSig[i];
        w2 = w2 - g2 * e0 * outPlant[i];

        // ---- second step (i+1) ----
        float u1 = w1 * refSig[i + 1] + w2 * outPlant[i + 1];
        outPlant[i + 2] = aTrue * outPlant[i + 1] + bTrue * u1;

        float e1 = outPlant[i + 2] - refSig[i + 1];
        w1 = w1 - g1 * e1 * refSig[i + 1];
        w2 = w2 - g2 * e1 * outPlant[i + 1];
    }

    // handle possible last iteration if nSteps is odd
    if (i < nSteps) {
        float u = w1 * refSig[i] + w2 * outPlant[i];
        outPlant[i + 1] = aTrue * outPlant[i] + bTrue * u;
        float e = outPlant[i + 1] - refSig[i];
        w1 = w1 - g1 * e * refSig[i];
        w2 = w2 - g2 * e * outPlant[i];
    }

    // print final adaptive parameters and a few output samples
    std::cout << "Final adaptive gains:\n";
    std::cout << "w1 = " << w1 << "\n";
    std::cout << "w2 = " << w2 << "\n\n";

    std::cout << "Sample of plant output vs reference:\n";
    for (int k = 0; k < 10; ++k) {
        std::cout << "k=" << k
                  << "  y=" << outPlant[k]
                  << "  r=" << refSig[k] << "\n";
    }

    return 0;
}
