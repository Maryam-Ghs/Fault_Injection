#include <iostream>
#include <climits>

/* LLM input variant 8: sparse-skewed */
int main() {
    // ---- generate sparse‑skewed data ----
    int n = 20;                                 // size of the test set
    int *rawIn  = new int[n];                   // raw input values (mostly zeros)
    int *rawOut = new int[n];                   // desired output values

    // Initialize all entries to zero (sparse background)
    for (int i = 0; i < n; ++i) {
        rawIn[i]  = 0;
        rawOut[i] = 0;
    }

    // Cluster a few non‑zero, extreme values near the end (skewed region)
    rawIn[15]  = INT_MAX;
    rawOut[15] = INT_MAX;
    rawIn[16]  = -INT_MAX;
    rawOut[16] = -INT_MAX;
    rawIn[17]  = 123456789;
    rawOut[17] = -123456789;
    rawIn[18]  = -987654321;
    rawOut[18] = 987654321;
    // One moderate value near the start to avoid a completely zero sum
    rawIn[2]   = 42;
    rawOut[2]  = 42;

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
    float gain = (float)(sumOut) / ((float)sumIn + eps);

    // ---- apply gain to inputs to produce tuned outputs ----
    float *tuned = new float[n];
    i = 0;
    for (; i < n; ++i) {
        // fused multiplication and cast
        tuned[i] = (float)rawIn[i] * gain;
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
