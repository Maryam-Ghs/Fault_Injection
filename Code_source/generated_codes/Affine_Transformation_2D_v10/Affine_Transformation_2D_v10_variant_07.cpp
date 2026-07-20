#include <iostream>
/* LLM input variant 7: reverse-adversarial */

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

    // points to transform (x, y) – reverse‑ordered and extreme values
    const int N = 8; // fixed size, known at compile time
    float px[N] = { 7.7f, -1.2f, 2.5f, -3.0f, 5.0f, 0.0f, 1e6f, -1e6f };
    float py[N] = { -6.6f, 3.3f, -1.5f, 4.0f, -2.0f, 0.0f, 1e-6f, -1e-6f };

    // output buffers
    float qx[N];
    float qy[N];

    // --- manual loop unrolling (process two points per iteration) ----------
    int i = 0;
    while (i + 1 < N) {
        // first point (i)
        qx[i] = a * px[i] + b * py[i] + tx; // fused: mul‑add‑add
        qy[i] = c * px[i] + d * py[i] + ty;

        // second point (i+1)
        qx[i + 1] = a * px[i + 1] + b * py[i + 1] + tx;
        qy[i + 1] = c * px[i + 1] + d * py[i + 1] + ty;

        i += 2;
    }
    // handle possible odd element (none for N=8, but keep for completeness)
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
