#include <iostream>

/* LLM input variant 8: sparse-skewed */

int main() {
    // version #10
    // 2D affine transformation test (manual loop unrolling, fused expressions, stack arrays)

    // --- deterministic input -------------------------------------------------
    // transformation matrix components
    float a = 1.2f;   // scale‑x
    float b = 0.5f;   // shear‑x
    float tx = 10.0f; // translate‑x
    float c = -0.3f;  // shear‑y
    float d = 0.8f;   // scale‑y
    float ty = -5.0f; // translate‑y

    // points to transform (x, y) – mostly zeros, few clustered non‑zero entries
    const int N = 20; // larger size to emphasize sparsity
    float px[N] = {
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f,   // empty region
        1.0f, 2.0f, 3.0f,            // clustered group
        0.0f, 0.0f, 0.0f, 0.0f,       // empty region
        -4.5f, -4.6f, -4.7f,          // another cluster (negative side)
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f // trailing zeros
    };
    float py[N] = {
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        1.0f, 2.0f, 3.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        -4.5f, -4.6f, -4.7f,
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f
    };

    // output buffers
    float qx[N];
    float qy[N];

    // --- manual loop unrolling (process two points per iteration) ----------
    int i = 0;
    while (i + 1 < N) {
        // first point (i)
        qx[i] = a * px[i] + b * py[i] + tx;
        qy[i] = c * px[i] + d * py[i] + ty;

        // second point (i+1)
        qx[i + 1] = a * px[i + 1] + b * py[i + 1] + tx;
        qy[i + 1] = c * px[i + 1] + d * py[i + 1] + ty;

        i += 2;
    }
    // handle possible odd element
    if (i < N) {
        qx[i] = a * px[i] + b * py[i] + tx;
        qy[i] = c * px[i] + d * py[i] + ty;
    }

    // --- print results -------------------------------------------------------
    std::cout << "Affine transformation results (x', y'):\n";
    for (int k = 0; k < N; ++k) {
        std::cout << "P" << k << ": (" << qx[k] << ", " << qy[k] << ")\n";
    }

    return 0;
}
