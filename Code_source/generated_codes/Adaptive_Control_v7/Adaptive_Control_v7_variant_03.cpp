#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

/* LLM input variant 3: zeros-and-ones */

int main() {
    // version #7 - adaptive control demonstration
    // deterministic input, no random seed needed
    // std::srand(static_cast<unsigned>(std::time(nullptr)));

    // simulation length
    int nSteps = 2000;

    // generate reference trajectory emphasizing zeros and ones
    std::vector<float> refSig(nSteps);
    for (int i = 0; i < nSteps; ++i) {
        // pattern: even indices -> 0.0, odd indices -> 1.0
        refSig[i] = (i % 2 == 0) ? 0.0f : 1.0f;
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
