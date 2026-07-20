/* LLM input variant 9: medium-deterministic-random */
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
    // Medium‑sized predefined input array (generated internally)
    const int dataSize = 20;
    float srcData[dataSize] = {
        0.12f, -0.45f, 0.78f, 1.23f, -1.50f,
        2.34f, -2.67f, 3.01f, -0.99f, 0.00f,
        4.56f, -3.33f, 2.22f, -1.11f, 5.55f,
        -4.44f, 6.66f, -5.55f, 7.77f, -6.66f
    };
    float dstData[dataSize];                     // stack‑allocated destination

    // Create a tuner object and run the adaptation
    Tuner adapter;
    adapter.adapt(srcData, dstData, dataSize);

    // Print the tuned results
    int idx = 0;
    while (idx < dataSize)                       // another while loop for output
    {
        std::printf("%.3f ", dstData[idx]);
        ++idx;
    }
    std::printf("\n");

    return 0;
}
