#include <iostream>
#include <climits>

/* LLM input variant 10: large-safe-stress */

int main() {
    // ---- generate large safe stress data ----
    const int n = 10000;                         // larger test set size
    int *rawIn  = new int[n];
    int *rawOut = new int[n];

    // Populate with a deterministic mix of values, including extremes
    for (int i = 0; i < n; ++i) {
        if (i == 0) {
            rawIn[i]  = 0;
            rawOut[i] = 0;
        } else if (i == 1) {
            rawIn[i]  = INT_MAX;
            rawOut[i] = INT_MAX;
        } else if (i == 2) {
            rawIn[i]  = INT_MIN;
            rawOut[i] = INT_MIN;
        } else if (i % 3 == 0) {
            rawIn[i]  = i;          // positive growing
            rawOut[i] = -i;         // opposite sign
        } else {
            rawIn[i]  = -i;         // negative growing
            rawOut[i] = i;          // opposite sign
        }
    }

    // ---- compute tuning factor (adapter gain) ----
    long long sumIn  = 0;
    long long sumOut = 0;
    for (int i = 0; i < n; ++i) {
        sumIn  += rawIn[i];
        sumOut += rawOut[i];
    }

    // Fuse epsilon addition to avoid division by zero without explicit branch
    float eps = 1e-6f;
    float gain = static_cast<float>(sumOut) / (static_cast<float>(sumIn) + eps);

    // ---- apply gain to inputs to produce tuned outputs ----
    float *tuned = new float[n];
    for (int i = 0; i < n; ++i) {
        tuned[i] = static_cast<float>(rawIn[i]) * gain;
    }

    // ---- print results (first 20 entries for brevity) ----
    std::cout << "Adapter gain: " << gain << "\n";
    std::cout << "Idx  RawIn   RawOut   Tuned\n";
    for (int i = 0; i < 20 && i < n; ++i) {
        std::cout << i << "    " << rawIn[i] << "    " << rawOut[i] << "    " << tuned[i] << "\n";
    }
    if (n > 20) {
        std::cout << " ... (remaining " << n - 20 << " entries omitted) ...\n";
    }

    // ---- cleanup ----
    delete[] rawIn;
    delete[] rawOut;
    delete[] tuned;
    return 0;
}
