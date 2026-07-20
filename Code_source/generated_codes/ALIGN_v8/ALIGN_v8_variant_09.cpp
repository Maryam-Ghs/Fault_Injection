#include <iostream>
using namespace std;

/* LLM input variant 9: medium-deterministic-random */

// version 8 – ALIGN demonstration
class AlignEngine {
public:
    AlignEngine(int length) : len(length) {
        src = new float[len];
        dst = new float[len];
        // generate medium‑sized deterministic pseudo‑random arrays
        unsigned int seed1 = 123456789;
        unsigned int seed2 = 987654321;
        for (int k = 0; k < len; ++k) {
            // simple linear congruential generator for reproducibility
            seed1 = (1664525u * seed1 + 1013904223u);
            seed2 = (22695477u * seed2 + 1u);
            src[k] = static_cast<float>((seed1 % 2000) - 1000) / 10.0f; // range roughly -100..100
            dst[k] = static_cast<float>((seed2 % 1500) - 750) / 10.0f;  // range roughly -75..75
        }
    }
    ~AlignEngine() {
        delete[] src;
        delete[] dst;
    }

    float execute() {
        // compute sum of absolute differences, manual loop unrolling
        float sum = 0.0f;
        int idx = 0;
        int bulk = len - (len % 4);

        // unrolled part using a while‑loop
        while (idx < bulk) {
            // load elements
            float v0 = src[idx]   - dst[idx];
            float v1 = src[idx+1] - dst[idx+1];
            float v2 = src[idx+2] - dst[idx+2];
            float v3 = src[idx+3] - dst[idx+3];

            // split into temporaries for absolute value
            float a0 = v0 >= 0.0f ? v0 : -v0;
            float a1 = v1 >= 0.0f ? v1 : -v1;
            float a2 = v2 >= 0.0f ? v2 : -v2;
            float a3 = v3 >= 0.0f ? v3 : -v3;

            // accumulate
            sum = sum + a0;
            sum = sum + a1;
            sum = sum + a2;
            sum = sum + a3;

            idx = idx + 4;               // advance by four
        }

        // handle the tail (if any) with a simple for‑loop
        for (; idx < len; ++idx) {
            float diff = src[idx] - dst[idx];
            float absDiff = diff >= 0.0f ? diff : -diff;
            sum = sum + absDiff;
        }

        return sum;
    }

    void showResult() {
        float res = execute();
        cout << "Sum of absolute differences = " << res << endl;
    }

private:
    int len;          // array length
    float* src;       // first heap array
    float* dst;       // second heap array
};

int main() {
    int dataSize = 53;                 // medium predefined size
    AlignEngine proc(dataSize);
    proc.showResult();
    return 0;
}
