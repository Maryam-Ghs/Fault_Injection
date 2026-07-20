#include <iostream>
#include <cmath>

/* LLM input variant 1: minimal-boundary */
int main() {
    // size of the problem (minimal nontrivial)
    int n = 1;

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

    // initialise parameters, gradients and moments
    w[0] = 0.5f;
    g[0] = 0.02f;
    m[0] = 0.0f;
    v[0] = 0.0f;

    // ---------- AdamW update (single element) ----------
    // update first moment and second moment
    m[0] = b1 * m[0] + (1.0f - b1) * g[0];
    v[0] = b2 * v[0] + (1.0f - b2) * g[0] * g[0];

    // bias‑corrected moments (reordered: compute denominator first)
    float denom0 = std::sqrt(v[0] / (1.0f - b2_pow)) + eps;
    float step0  = (m[0] / (1.0f - b1_pow)) / denom0 + wd * w[0];

    // final parameter update
    w[0] -= lr * step0;

    // ---------- output ----------
    std::cout << "Updated parameters:\n";
    std::cout << w[0] << "\n";

    // clean up
    delete[] w;
    delete[] g;
    delete[] m;
    delete[] v;

    return 0;
}
