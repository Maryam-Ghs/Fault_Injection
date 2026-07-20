/* LLM input variant 8: sparse-skewed */
#include <iostream>
using namespace std;

// version 8 – ALIGN demonstration
class AlignEngine {
public:
    AlignEngine(int length) : len(length) {
        src = new float[len];
        dst = new float[len];
        // initialize all elements to zero (sparse baseline)
        for (int k = 0; k < len; ++k) {
            src[k] = 0.0f;
            dst[k] = 0.0f;
        }
        // populate a few scattered non‑zero entries (skewed distribution)
        if (len > 5) {
            src[5]  = 10.0f;
            dst[5]  = 2.0f;
        }
        if (len > 50) {
            src[50] = 5.5f;
            dst[50] = 0.5f;
        }
        if (len > 99) {
            src[99] = 3.3f;
            dst[99] = 1.1f;
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
    int dataSize = 100;                 // larger size with sparse data
    AlignEngine proc(dataSize);
    proc.showResult();
    return 0;
}
