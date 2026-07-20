#include <iostream>
#include <cmath>

/* LLM input variant 10: large-safe-stress */
int main() {
    // size of the problem (large but safe)
    const int n = 1000;

    // allocate all vectors on the heap
    float* w = new float[n];   // parameters
    float* g = new float[n];   // gradients
    float* m = new float[n];   // first moment
    float* v = new float[n];   // second moment

    // hyper‑parameters (all float)
    const float lr = 0.001f;
    const float b1 = 0.9f;
    const float b2 = 0.999f;
    const float eps = 1e-8f;
    const float wd = 0.01f;           // weight decay

    // bias‑correction powers (t = 1 for a single update)
    const float b1_pow = b1;          // beta1^t
    const float b2_pow = b2;          // beta2^t

    // initialise parameters, gradients and moments deterministically
    for (int i = 0; i < n; ++i) {
        w[i] = 0.5f - 0.001f * i;            // decreasing values
        g[i] = (i % 2 == 0) ? 0.02f : -0.02f; // alternating signs
        m[i] = 0.0f;
        v[i] = 0.0f;
    }

    // ---------- AdamW update (loop) ----------
    for (int i = 0; i < n; ++i) {
        m[i] = b1 * m[i] + (1.0f - b1) * g[i];
        v[i] = b2 * v[i] + (1.0f - b2) * g[i] * g[i];
        float denom = std::sqrt(v[i] / (1.0f - b2_pow)) + eps;
        float step  = (m[i] / (1.0f - b1_pow)) / denom + wd * w[i];
        w[i] -= lr * step;
    }

    // ---------- output ----------
    std::cout << "Updated parameters (first 10):\n";
    for (int i = 0; i < 10; ++i) {
        std::cout << w[i] << (i == 9 ? "\n" : " ");
    }

    // clean up
    delete[] w;
    delete[] g;
    delete[] m;
    delete[] v;

    return 0;
}
