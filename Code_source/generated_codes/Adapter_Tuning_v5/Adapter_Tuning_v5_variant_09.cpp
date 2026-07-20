#include <iostream>
#include <iomanip>
#include <random>

/* LLM input variant 9: medium-deterministic-random */

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
    // ----- generate large deterministic arrays on the stack -----
    const int N = 20000;               // size of test data
    float srcArr[N];
    float dstArr[N];

    // Deterministic pseudo‑random generator with fixed seed
    std::mt19937 rng(987654321);
    std::uniform_int_distribution<int> rawDist(0, 200);      // 0 .. 200
    std::uniform_int_distribution<int> signDist(0, 1);    // 0 or 1
    std::uniform_int_distribution<int> noiseDist(0, 1000);   // 0 .. 1000

    for (int idx = 0; idx < N; ++idx) {
        int raw = rawDist(rng);
        int sign = signDist(rng) == 0 ? -1 : 1;
        srcArr[idx] = static_cast<float>(raw * sign);
    }

    // Simulate an unknown scaling factor (e.g., 2.73) and add tiny noise
    float trueScale = 2.73f;
    for (int idx = 0; idx < N; ++idx) {
        // noise in [-0.5, 0.5]
        float noise = (static_cast<float>(noiseDist(rng)) - 500.0f) * 0.001f;
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
