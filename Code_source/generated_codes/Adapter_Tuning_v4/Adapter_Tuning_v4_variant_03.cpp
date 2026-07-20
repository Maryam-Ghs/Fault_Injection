#include <iostream>
#include <climits>

/* LLM input variant 3: zeros-and-ones */
int main() {
    // ---- generate edge‑case heavy data ----
    int n = 12;                                 // size of the test set
    int *rawIn  = new int[n];                   // raw input values (could be zero, extreme, etc.)
    int *rawOut = new int[n];                   // desired output values

    // Populate with zeros, ones, and identity mappings
    rawIn[0]  = 0;
    rawOut[0] = 0;
    rawIn[1]  = 1;
    rawOut[1] = 1;
    rawIn[2]  = 0;
    rawOut[2] = 0;
    rawIn[3]  = 1;
    rawOut[3] = 1;
    rawIn[4]  = 0;
    rawOut[4] = 0;
    rawIn[5]  = 1;
    rawOut[5] = 1;
    rawIn[6]  = 0;
    rawOut[6] = 0;
    rawIn[7]  = 1;
    rawOut[7] = 1;
    rawIn[8]  = 0;
    rawOut[8] = 0;
    rawIn[9]  = 1;
    rawOut[9] = 1;
    rawIn[10] = 0;
    rawOut[10] = 0;
    rawIn[11] = 1;
    rawOut[11] = 1;

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
