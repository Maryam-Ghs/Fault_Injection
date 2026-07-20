#include <iostream>
#include <cstdlib>
#include <ctime>

/* LLM input variant 7: reverse-adversarial */

class AdapterTuner {
public:
    // Simple linear adapter: y = mul * x + add
    float mul;   // multiplier (gain)
    float add;   // additive offset

    AdapterTuner() : mul(1.0f), add(0.0f) {}

    // Tune parameters using a basic LMS-like rule
    void tune(const float inArr[], const float tarArr[], int sz, int epochs, float step) {
        int epoch = 0;
        while (epoch < epochs) {
            int idx = 0;
            while (idx < sz) {
                // ---- forward pass ----
                float inp   = inArr[idx];
                float wMul  = mul;          // temp copy
                float wAdd  = add;          // temp copy
                float pred  = wMul * inp + wAdd;

                // ---- error computation ----
                float targ   = tarArr[idx];
                float diff   = targ - pred; // error

                // ---- parameter updates (LMS) ----
                float deltaM = step * diff * inp;
                float deltaA = step * diff;

                // apply updates
                mul = mul + deltaM;
                add = add + deltaA;

                idx = idx + 1; // next element
            }
            epoch = epoch + 1; // next epoch
        }
    }

    // Apply the adapted model to an array
    void apply(const float src[], float dst[], int sz) const {
        for (int i = 0; i < sz; ++i) {
            float v = src[i];
            float m = mul;
            float a = add;
            dst[i] = m * v + a;
        }
    }
};

int main() {
    // --- generate deterministic adversarial data ---
    const int N = 64;                     // medium size array
    float input[N];
    float target[N];

    // Create a reverse-ordered input ranging from +5 down to -5
    for (int i = 0; i < N; ++i) {
        input[i] = 5.0f - i * (10.0f / (N - 1)); // descending linearly
    }

    // True relationship: y = 2.3 * x - 0.7  + adversarial noise
    // Noise alternates sign to maximize initial error magnitude
    for (int i = 0; i < N; ++i) {
        float noise = (i % 2 == 0) ? -0.1f : 0.1f; // worst-case alternating noise
        target[i] = 2.3f * input[i] - 0.7f + noise;
    }

    // --- run tuner ---
    AdapterTuner tuner;
    tuner.tune(input, target, N, 30, 0.05f);   // 30 epochs, step size 0.05

    // --- produce output using tuned parameters ---
    float out[N];
    tuner.apply(input, out, N);

    // --- print results ---
    std::cout << "Tuned multiplier (gain): " << tuner.mul << "\n";
    std::cout << "Tuned additive offset:   " << tuner.add << "\n\n";

    std::cout << "Idx  Input   Target    Output   Error\n";
    for (int i = 0; i < N; ++i) {
        float err = target[i] - out[i];
        std::cout << i << "  "
                  << input[i] << "  "
                  << target[i] << "  "
                  << out[i] << "  "
                  << err << "\n";
    }
    return 0;
}
