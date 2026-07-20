#include <iostream>
#include <climits>

/* LLM input variant 1: minimal-boundary */

int main() {
    // ---- generate minimal boundary data ----
    int n = 2;                                 // smallest nontrivial size
    int *rawIn  = new int[n];
    int *rawOut = new int[n];

    // Populate with boundary extremes
    rawIn[0]  = INT_MIN;
    rawOut[0] = INT_MIN;
    rawIn[1]  = INT_MAX;
    rawOut[1] = INT_MAX;

    // ---- compute tuning factor (adapter gain) ----
    long long sumIn  = 0;
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
