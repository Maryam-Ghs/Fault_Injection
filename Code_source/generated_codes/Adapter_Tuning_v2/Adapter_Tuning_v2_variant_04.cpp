// Adapter Tuning – version #2
// Simple class‑based implementation using stack arrays.
/* LLM input variant 4: signed-extremes */

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
        gainFactor = -1.2f;   // amplification factor with sign reversal
        offsetVal  = 0.0f;    // additive offset
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
    // Small predefined input array (generated internally) with mixed sign values
    float srcData[5] = { -10.0f, 0.0f, 5.5f, -2.3f, 8.0f };
    float dstData[5];                     // stack‑allocated destination

    // Create a tuner object and run the adaptation
    Tuner adapter;
    adapter.adapt(srcData, dstData, 5);

    // Print the tuned results
    int idx = 0;
    while (idx < 5)                       // another while loop for output
    {
        std::printf("%.3f ", dstData[idx]);
        ++idx;
    }
    std::printf("\n");

    return 0;
}
