#include <iostream>
#include <climits>

/* LLM input variant 2: small-diverse */

int main() {
    // ---- generate edge‑case heavy data ----
    int n = 9;                                 // size of the test set
    int *rawIn  = new int[n];                   // raw input values (could be zero, extreme, etc.)
    int *rawOut = new int[n];                   // desired output values

    // Populate with a mix of ordinary diverse values
    rawIn[0]  = 5;
    rawOut[0] = 10;
    rawIn[1]  = -3;
    rawOut[1] = -6;
    rawIn[2]  = 10;
    rawOut[2] = 20;
    rawIn[3]  = 0;
    rawOut[3] = 0;
    rawIn[4]  = 7;
    rawOut[4] = 14;
    rawIn[5]  = -2;
    rawOut[5] = -4;
    rawIn[6]  = 15;
    rawOut[6] = 30;
    rawIn[7]  = -8;
    rawOut[7] = -16;
    rawIn[8]  = 4;
    rawOut[8] = 8;

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
