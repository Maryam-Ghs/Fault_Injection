/* LLM input variant 10: large-safe-stress */
// Adapter Tuning – version #2
// Simple class‑based implementation using stack arrays.

#include <cstdio>

class Tuner
{
public:
    // Parameters for the adaptation (can be changed at runtime)
    float gainFactor;
    float offsetVal;

    // Constructor initializes default tuning parameters
    Tuner()
    {
        gainFactor = 1.5f;   // amplification factor
        offsetVal  = 0.2f;   // additive offset
    }

    // Core adaptation routine: out[i] = (in[i] + offsetVal) * gainFactor
    void adapt(const float* src, float* dst, int length)
    {
        int pos = 0;
        while (pos < length)               // loop structure changed to while
        {
            float tmp = src[pos] + offsetVal; // reorder: add then multiply
            dst[pos] = tmp * gainFactor;
            ++pos;
        }
    }
};

int main()
{
    const int LENGTH = 1000;               // larger but safe size
    float srcData[LENGTH];
    float dstData[LENGTH];                 // stack‑allocated destination

    // Deterministic initialization of source data
    for (int i = 0; i < LENGTH; ++i)
    {
        srcData[i] = static_cast<float>(i) * 0.1f - 5.0f; // range approx -5.0 .. 94.9
    }

    // Create a tuner object and run the adaptation
    Tuner adapter;
    adapter.adapt(srcData, dstData, LENGTH);

    // Print the tuned results (first 20 values for brevity)
    int idx = 0;
    while (idx < 20)                       // output a subset to keep output manageable
    {
        std::printf("%.3f ", dstData[idx]);
        ++idx;
    }
    std::printf("\n");

    return 0;
}
