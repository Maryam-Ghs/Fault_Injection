#include <iostream>
/* LLM input variant 8: sparse-skewed */

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
    // --- generate deterministic sparse-skewed data ---
    const int N = 64;                     // medium size array
    float input[N];
    float target[N];

    for (int i = 0; i < N; ++i) {
        // Create two small clusters of non‑zero values, rest are zero
        if (i < 5) {
            // First cluster: modest increasing values
            input[i] = static_cast<float>(i + 1) * 0.1f; // 0.1, 0.2, ..., 0.5
        } else if (i >= 20 && i < 25) {
            // Second cluster: larger spaced values
            input[i] = static_cast<float>(i - 19) * 0.2f; // 0.2, 0.4, ..., 1.0
        } else {
            input[i] = 0.0f; // sparse region
        }

        // True relationship with tiny deterministic noise
        float base = 2.3f * input[i] - 0.7f;
        float noise = (i % 3 == 0) ? 0.05f : -0.05f; // alternating small bias
        target[i] = base + noise;
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
