#include <iostream>
#include <climits>

/* LLM input variant 9: medium-deterministic-random */

int main() {
    // ---- generate medium‑size deterministic pseudo‑random data ----
    const int n = 20;                              // size of the test set
    int *rawIn  = new int[n];                       // raw input values
    int *rawOut = new int[n];                       // desired output values

    // Simple linear congruential generator with fixed seed
    unsigned int seed = 12345;
    for (int i = 0; i < n; ++i) {
        seed = seed * 1103515245u + 12345u;
        // map to range [-1'000'000, 1'000'000]
        int val = static_cast<int>((seed >> 1) % 2000001u) - 1000000;
        rawIn[i] = val;
        // produce a deterministic transformation for expected output
        rawOut[i] = -2 * val;
    }

    // ---- compute tuning factor (adapter gain) ----
    long long sumIn  = 0;      // use long long only for intermediate accumulation
    long long sumOut = 0;
    int i = 0;
    while (i < n) {
        sumIn  += rawIn[i];
        sumOut += rawOut[i];
        ++i;
    }

    // Fuse epsilon addition to avoid division by zero without explicit branch
    float eps = 1e-6f;
    float gain = static_cast<float>(sumOut) / (static_cast<float>(sumIn) + eps);

    // ---- apply gain to inputs to produce tuned outputs ----
    float *tuned = new float[n];
    i = 0;
    for (; i < n; ++i) {
        // fused multiplication and cast
        tuned[i] = static_cast<float>(rawIn[i]) * gain;
    }

    // ---- print results ----
    std::cout << "Adapter gain: " << gain << "\n";
    std::cout << "Idx  RawIn   RawOut   Tuned\n";
    for (i = 0; i < n; ++i) {
        std::cout << i << "    " << rawIn[i] << "    " << rawOut[i] << "    " << tuned[i] << "\n";
    }

    // ---- cleanup ----
    delete[] rawIn;
    delete[] rawOut;
    delete[] tuned;
    return 0;
}
