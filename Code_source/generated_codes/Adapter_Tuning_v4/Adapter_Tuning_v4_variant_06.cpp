#include <iostream>
#include <climits>

/* LLM input variant 6: ordered-structured */
int main() {
    // ---- generate ordered-structured data ----
    int n = 12;                                 // size of the test set
    int *rawIn  = new int[n];                   // raw input values (ordered)
    int *rawOut = new int[n];                   // desired output values (mirrored)

    // Populate with a symmetric, sorted pattern
    rawIn[0]  = -5;   rawOut[0] = 5;
    rawIn[1]  = -4;   rawOut[1] = 4;
    rawIn[2]  = -3;   rawOut[2] = 3;
    rawIn[3]  = -2;   rawOut[3] = 2;
    rawIn[4]  = -1;   rawOut[4] = 1;
    rawIn[5]  = 0;    rawOut[5] = 0;
    rawIn[6]  = 1;    rawOut[6] = -1;
    rawIn[7]  = 2;    rawOut[7] = -2;
    rawIn[8]  = 3;    rawOut[8] = -3;
    rawIn[9]  = 4;    rawOut[9] = -4;
    rawIn[10] = 5;    rawOut[10] = -5;
    rawIn[11] = 6;    rawOut[11] = -6;

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
