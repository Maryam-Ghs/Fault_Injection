/* LLM input variant 7: reverse-adversarial */
#include <iostream>
#include <climits>

int main() {
    // ---- generate edge‑case heavy data ----
    int n = 12;                                 // size of the test set
    int *rawIn  = new int[n];                   // raw input values (could be zero, extreme, etc.)
    int *rawOut = new int[n];                   // desired output values

    // Populate with reverse‑ordered, adversarial values that cancel each other
    rawIn[0]  = INT_MAX;
    rawOut[0] = INT_MAX;
    rawIn[1]  = 123456789;
    rawOut[1] = -123456789;
    rawIn[2]  = 99999999;
    rawOut[2] = -99999999;
    rawIn[3]  = 42;
    rawOut[3] = -42;
    rawIn[4]  = 1;
    rawOut[4] = -1;
    rawIn[5]  = 0;
    rawOut[5] = 0;
    rawIn[6]  = -1;
    rawOut[6] = 1;
    rawIn[7]  = -42;
    rawOut[7] = 42;
    rawIn[8]  = -99999999;
    rawOut[8] = 99999999;
    rawIn[9]  = -123456789;
    rawOut[9] = 123456789;
    rawIn[10] = -INT_MAX;
    rawOut[10] = -INT_MAX;
    rawIn[11] = 0;
    rawOut[11] = 0;

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
