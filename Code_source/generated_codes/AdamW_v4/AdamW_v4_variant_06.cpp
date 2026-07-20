#include <iostream>
#include <cmath>

/* LLM input variant 6: ordered-structured */

int main() {
    // size of the problem
    int n = 10;

    // allocate all vectors on the heap
    float* w = new float[n];   // parameters
    float* g = new float[n];   // gradients
    float* m = new float[n];   // first moment
    float* v = new float[n];   // second moment

    // hyper‑parameters (all float)
    float lr = 0.001f;
    float b1 = 0.9f;
    float b2 = 0.999f;
    float eps = 1e-8f;
    float wd = 0.01f;           // weight decay

    // bias‑correction powers (t = 1 for a single update)
    float b1_pow = b1;          // beta1^t
    float b2_pow = b2;          // beta2^t

    // initialise parameters (sorted ascending) and gradients (nearly sorted)
    w[0] = -0.9f; w[1] = -0.5f; w[2] = -0.1f; w[3] = 0.0f; w[4] = 0.2f;
    w[5] = 0.4f; w[6] = 0.6f; w[7] = 0.8f; w[8] = 1.0f; w[9] = 1.2f;

    g[0] = -0.05f; g[1] = -0.04f; g[2] = -0.03f; g[3] = -0.02f; g[4] = -0.01f;
    g[5] = 0.0f;   g[6] = 0.01f; g[7] = 0.02f;  g[8] = 0.03f;  g[9] = 0.04f;

    for (int i = 0; i < n; ++i) {
        m[i] = 0.0f;
        v[i] = 0.0f;
    }

    // ---------- AdamW update (loop version) ----------
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
    std::cout << "Updated parameters:\n";
    for (int i = 0; i < n; ++i) {
        std::cout << w[i];
        if (i + 1 < n) std::cout << " ";
    }
    std::cout << "\n";

    // clean up
    delete[] w;
    delete[] g;
    delete[] m;
    delete[] v;

    return 0;
}
