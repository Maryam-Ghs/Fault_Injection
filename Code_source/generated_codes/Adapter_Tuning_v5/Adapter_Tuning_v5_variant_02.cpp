#include <iostream>
#include <cstdlib>
#include <ctime>
#include <iomanip>

/* LLM input variant 2: small-diverse */

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
    // ----- generate small deterministic arrays on the stack -----
    const int N = 15;               // size of test data
    float srcArr[N] = {
        -50.5f, 0.0f, 23.7f, -1.2f, 99.9f,
        -33.3f, 12.0f, -45.6f, 8.8f, 0.001f,
        -0.999f, 50.0f, -20.0f, 5.5f, 3.3f
    };
    float noise[N] = {
        0.012f, -0.005f, 0.300f, -0.200f, 0.050f,
        -0.075f, 0.000f, 0.123f, -0.045f, 0.001f,
        -0.010f, 0.200f, -0.150f, 0.025f, -0.005f
    };
    float dstArr[N];

    // Simulate an unknown scaling factor and add tiny noise
    float trueScale = 1.57f;
    int idx = 0;
    while (idx < N) {
        dstArr[idx] = trueScale * srcArr[idx] + noise[idx];
        ++idx;
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
