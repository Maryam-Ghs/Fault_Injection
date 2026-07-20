// Adapter Tuning – version #2
// Simple class‑based implementation using stack arrays.
/* LLM input variant 3: zeros-and-ones */

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
        gainFactor = 1.0f;   // identity factor
        offsetVal  = 0.0f;   // no offset
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
    // Small predefined input array (generated internally) emphasizing zeros and ones
    float srcData[5] = { 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };
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
