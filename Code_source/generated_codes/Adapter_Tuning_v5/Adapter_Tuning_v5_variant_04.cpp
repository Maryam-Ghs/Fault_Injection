#include <iostream>
#include <cstdlib>
#include <ctime>
#include <iomanip>

/* LLM input variant 4: signed-extremes */

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
    // ----- generate deterministic mixed-sign arrays on the stack -----
    const int N = 20000;               // size of test data
    float srcArr[N];
    float dstArr[N];

    // Deterministic pattern: -50, 0, 50 repeating, ensuring mix of negative, zero, positive
    for (int idx = 0; idx < N; ++idx) {
        int pattern = idx % 3; // 0,1,2
        float value = (pattern - 1) * 50.0f; // -50, 0, 50
        srcArr[idx] = value;
    }

    // Simulate an unknown scaling factor (e.g., 2.73) and add deterministic tiny noise
    float trueScale = 2.73f;
    for (int idx = 0; idx < N; ++idx) {
        // Noise pattern cycles through -0.15, -0.10, -0.05, 0, 0.05, 0.10, 0.15
        float noise = ((idx % 7) - 3) * 0.05f;
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
