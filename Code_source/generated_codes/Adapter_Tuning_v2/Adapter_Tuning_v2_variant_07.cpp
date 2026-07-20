/* LLM input variant 7: reverse-adversarial */
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
    // Reverse‑ordered, adversarial input array (generated internally)
    const int N = 10;
    float srcData[N] = {
        1000.0f, 800.0f, 600.0f, 400.0f, 200.0f,
        -200.0f, -400.0f, -600.0f, -800.0f, -1000.0f
    };
    float dstData[N];                     // stack‑allocated destination

    // Create a tuner object and run the adaptation
    Tuner adapter;
    adapter.adapt(srcData, dstData, N);

    // Print the tuned results
    int idx = 0;
    while (idx < N)                       // another while loop for output
    {
        std::printf("%.3f ", dstData[idx]);
        ++idx;
    }
    std::printf("\n");

    return 0;
}
