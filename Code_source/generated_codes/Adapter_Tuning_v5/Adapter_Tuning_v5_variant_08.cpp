#include <iostream>
#include <cstdlib>
#include <ctime>
#include <iomanip>

/* LLM input variant 8: sparse-skewed */

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
    // ----- generate sparse, skewed input arrays on the stack -----
    const int N = 20000;               // size of test data
    float srcArr[N];
    float dstArr[N];

    std::srand(42); // deterministic seed
    for (int i = 0; i < N; ++i) {
        if (i % 500 == 0) {
            // Sparse non‑zero values with larger magnitude
            int mag = 50 + std::rand() % 51;               // 50 .. 100
            int sign = (std::rand() & 1) ? 1 : -1;        // -1 or 1
            srcArr[i] = static_cast<float>(mag * sign);
        } else if (i >= 9500 && i < 9600) {
            // Cluster of non‑zero values
            int raw = std::rand() % 201;                 // 0 .. 200
            int sign = (std::rand() & 1) ? 1 : -1;
            srcArr[i] = static_cast<float>(raw * sign);
        } else {
            // Mostly zeros
            srcArr[i] = 0.0f;
        }
    }

    // Simulate an unknown scaling factor (e.g., 2.73) and add tiny noise
    float trueScale = 2.73f;
    for (int i = 0; i < N; ++i) {
        // noise in [-0.5, 0.5]
        float noise = (static_cast<float>(std::rand() % 1001) - 500.0f) * 0.001f;
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
