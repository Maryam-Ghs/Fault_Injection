#include <iostream>
#include <iomanip>

/* LLM input variant 6: ordered-structured */

class AdapterTuner {
public:
    // Tune a linear adapter (scale) so that dst ≈ scale * src
    // Returns the scale factor.
    float tune(const float* src, const float* dst, int len) {
        // Compute Σ(src[i] * dst[i]) and Σ(src[i] * src[i]) without branches
        float accNum = 0.0f;
        float accDen = 0.0f;
        int pos = 0;
        while (pos < len) {
            float a = src[pos];
            float b = dst[pos];
            accNum += a * b;
            accDen += a * a;
            ++pos;
        }
        // Avoid division by zero by adding a tiny epsilon (branch‑free)
        float eps = 1e-6f;
        return accNum / (accDen + eps);
    }

    // Compute mean squared error after applying the scale
    float mse(const float* src, const float* dst, int len, float scale) {
        float errSum = 0.0f;
        int i = 0;
        while (i < len) {
            float diff = dst[i] - scale * src[i];
            errSum += diff * diff;
            ++i;
        }
        // Reorder: division after multiplication
        return errSum / static_cast<float>(len);
    }
};

int main() {
    // ----- generate large ordered, structured arrays on the stack -----
    const int N = 20000;               // size of test data
    float srcArr[N];
    float dstArr[N];

    // Create a symmetric, sorted source array: values from -N/2 to N/2-1
    for (int i = 0; i < N; ++i) {
        srcArr[i] = static_cast<float>(i - N / 2);
    }

    // Simulate a known scaling factor and add deterministic tiny noise
    float trueScale = 2.73f;
    for (int i = 0; i < N; ++i) {
        // Deterministic noise in [-0.05, 0.05] using a simple pattern
        float noise = ((i % 101) - 50) * 0.001f; // cycles every 101 elements
        dstArr[i] = trueScale * srcArr[i] + noise;
    }

    // ----- perform adapter tuning -----
    AdapterTuner tuner;
    float estimatedScale = tuner.tune(srcArr, dstArr, N);
    float error = tuner.mse(srcArr, dstArr, N, estimatedScale);

    // ----- output results -----
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "Estimated scale : " << estimatedScale << '\n';
    std::cout << "True scale      : " << trueScale << '\n';
    std::cout << "Mean squared error : " << error << '\n';
    return 0;
}
