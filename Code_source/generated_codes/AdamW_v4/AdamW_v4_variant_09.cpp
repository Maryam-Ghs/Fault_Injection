/* LLM input variant 9: medium-deterministic-random */
#include <iostream>
#include <cmath>
#include <cstdint>

int main() {
    // size of the problem (medium)
    const int n = 8;

    // allocate all vectors on the heap
    float* w = new float[n];   // parameters
    float* g = new float[n];   // gradients
    float* m = new float[n];   // first moment
    float* v = new float[n];   // second moment

    // hyper‑parameters (all float)
    const float lr  = 0.001f;
    const float b1  = 0.9f;
    const float b2  = 0.999f;
    const float eps = 1e-8f;
    const float wd  = 0.01f;           // weight decay

    // bias‑correction powers (t = 1 for a single update)
    const float b1_pow = b1;          // beta1^t
    const float b2_pow = b2;          // beta2^t

    // deterministic pseudo‑random generator (LCG)
    const uint64_t MOD = 1ULL << 31;
    const uint64_t A   = 1103515245ULL;
    const uint64_t C   = 12345ULL;
    uint64_t seed = 1234567ULL;  // fixed seed for reproducibility

    auto next_float = [&]() -> float {
        seed = (A * seed + C) % MOD;
        // map to [-1, 1)
        return static_cast<float>(seed) / static_cast<float>(MOD) * 2.0f - 1.0f;
    };

    // initialise parameters and gradients with deterministic pseudo‑random values
    for (int i = 0; i < n; ++i) {
        w[i] = next_float();   // parameters in [-1,1)
        g[i] = next_float();   // gradients in [-1,1)
        m[i] = 0.0f;
        v[i] = 0.0f;
    }

    // ---------- AdamW update (looped) ----------
    for (int i = 0; i < n; ++i) {
        m[i] = b1 * m[i] + (1.0f - b1) * g[i];
        v[i] = b2 * v[i] + (1.0f - b2) * g[i] * g[i];
    }

    // bias‑corrected moments and parameter update
    for (int i = 0; i < n; ++i) {
        float denom = std::sqrt(v[i] / (1.0f - b2_pow)) + eps;
        float step  = (m[i] / (1.0f - b1_pow)) / denom + wd * w[i];
        w[i] -= lr * step;
    }

    // ---------- output ----------
    std::cout << "Updated parameters:\\n";
    for (int i = 0; i < n; ++i) {
        std::cout << w[i];
        if (i + 1 < n) std::cout << ' ';
    }
    std::cout << '\\n';

    // clean up
    delete[] w;
    delete[] g;
    delete[] m;
    delete[] v;

    return 0;
}
