/* LLM input variant 7: reverse-adversarial */
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <iomanip>

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
    // ----- generate deterministic reverse‑ordered arrays on the stack -----
    const int N = 20000;               // size of test data
    float srcArr[N];
    float dstArr[N];

    // Fill srcArr with values descending from 100 to -100 (reverse‑ordered)
    for (int idx = 0; idx < N; ++idx) {
        srcArr[idx] = 100.0f - idx * (200.0f / static_cast<float>(N - 1));
    }

    // Simulate an unknown scaling factor (e.g., 2.73) and add deterministic pseudo‑noise
    float trueScale = 2.73f;
    for (int idx = 0; idx < N; ++idx) {
        // deterministic noise in [-0.5, 0.5] using a simple sawtooth pattern
        float noise = (static_cast<float>(idx % 1001) - 500.0f) * 0.001f;
        dstArr[idx] = trueScale * srcArr[idx] + noise;
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
