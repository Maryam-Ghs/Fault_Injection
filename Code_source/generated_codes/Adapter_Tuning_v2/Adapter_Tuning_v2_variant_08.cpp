// Adapter Tuning – version #2
// Simple class‑based implementation using stack arrays.
/* LLM input variant 8: sparse-skewed */

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
    // Sparse, skewed input array (mostly zeros with clustered non‑zero values)
    const int dataSize = 20;
    float srcData[dataSize] = {
        0.5f, 1.0f, -0.3f,            // clustered non‑zero at start
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f,
        2.5f, -1.5f, 0.8f,           // another cluster in the middle
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f
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
