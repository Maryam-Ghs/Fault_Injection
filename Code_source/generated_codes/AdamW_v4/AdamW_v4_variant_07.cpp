#include <iostream>
#include <cmath>

/* LLM input variant 7: reverse-adversarial */

int main() {
    // size of the problem
    int n = 4;

    // allocate all vectors on the heap
    float* w = new float[n];   // parameters
    float* g = new float[n];   // gradients
    float* m = new float[n];   // first moment
    float* v = new float[n];   // second moment

    // hyper‑parameters (all float) – chosen for adversarial effect
    float lr = 10.0f;           // very large learning rate
    float b1 = 0.01f;           // near‑zero beta1
    float b2 = 0.9999f;         // beta2 extremely close to 1
    float eps = 1e-12f;         // extremely small epsilon
    float wd = 0.5f;            // heavy weight decay

    // bias‑correction powers (t = 1 for a single update)
    float b1_pow = b1;          // beta1^t
    float b2_pow = b2;          // beta2^t

    // initialise parameters, gradients and moments (reverse‑ordered, large magnitudes)
    w[0] = -1000.0f;  w[1] = 800.0f;  w[2] = -600.0f;  w[3] = 400.0f;
    g[0] = -0.5f;    g[1] = 0.4f;    g[2] = -0.3f;    g[3] = 0.2f;
    m[0] = 0.0f;     m[1] = 0.0f;    m[2] = 0.0f;    m[3] = 0.0f;
    v[0] = 0.0f;     v[1] = 0.0f;    v[2] = 0.0f;    v[3] = 0.0f;

    // ---------- AdamW update (manual unrolling, reversed order) ----------
    // index 3
    m[3] = b1 * m[3] + (1.0f - b1) * g[3];
    v[3] = b2 * v[3] + (1.0f - b2) * g[3] * g[3];
    // index 2
    m[2] = b1 * m[2] + (1.0f - b1) * g[2];
    v[2] = b2 * v[2] + (1.0f - b2) * g[2] * g[2];
    // index 1
    m[1] = b1 * m[1] + (1.0f - b1) * g[1];
    v[1] = b2 * v[1] + (1.0f - b2) * g[1] * g[1];
    // index 0
    m[0] = b1 * m[0] + (1.0f - b1) * g[0];
    v[0] = b2 * v[0] + (1.0f - b2) * g[0] * g[0];

    // bias‑corrected moments (reversed computation order)
    float denom3 = std::sqrt(v[3] / (1.0f - b2_pow)) + eps;
    float step3  = (m[3] / (1.0f - b1_pow)) / denom3 + wd * w[3];
    float denom2 = std::sqrt(v[2] / (1.0f - b2_pow)) + eps;
    float step2  = (m[2] / (1.0f - b1_pow)) / denom2 + wd * w[2];
    float denom1 = std::sqrt(v[1] / (1.0f - b2_pow)) + eps;
    float step1  = (m[1] / (1.0f - b1_pow)) / denom1 + wd * w[1];
    float denom0 = std::sqrt(v[0] / (1.0f - b2_pow)) + eps;
    float step0  = (m[0] / (1.0f - b1_pow)) / denom0 + wd * w[0];

    // final parameter update (reversed order)
    w[3] -= lr * step3;
    w[2] -= lr * step2;
    w[1] -= lr * step1;
    w[0] -= lr * step0;

    // ---------- output ----------
    std::cout << "Updated parameters:\n";
    std::cout << w[0] << " " << w[1] << " " << w[2] << " " << w[3] << "\n";

    // clean up
    delete[] w;
    delete[] g;
    delete[] m;
    delete[] v;

    return 0;
}
