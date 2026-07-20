#include <iostream>
#include <climits>

/* LLM input variant 4: signed-extremes */

int main() {
    // ---- generate edge‑case heavy data ----
    int n = 14;                                 // size of the test set
    int *rawIn  = new int[n];                   // raw input values (could be zero, extreme, etc.)
    int *rawOut = new int[n];                   // desired output values

    // Populate with a mix of edge cases
    rawIn[0]  = 0;
    rawOut[0] = 0;
    rawIn[1]  = INT_MAX;
    rawOut[1] = INT_MAX;
    rawIn[2]  = INT_MIN;
    rawOut[2] = INT_MIN;
    rawIn[3]  = 123;
    rawOut[3] = -123;
    rawIn[4]  = -456;
    rawOut[4] = 456;
    rawIn[5]  = 7890;
    rawOut[5] = -7890;
    rawIn[6]  = -9876;
    rawOut[6] = 9876;
    rawIn[7]  = 1;
    rawOut[7] = -1;
    rawIn[8]  = -1;
    rawOut[8] = 1;
    rawIn[9]  = 42;
    rawOut[9] = 42;
    rawIn[10] = -42;
    rawOut[10]= -42;
    rawIn[11] = INT_MAX / 2;
    rawOut[11]= INT_MAX / 2;
    rawIn[12] = INT_MIN / 2;
    rawOut[12]= INT_MIN / 2;
    rawIn[13] = 0;
    rawOut[13]= 0;

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
