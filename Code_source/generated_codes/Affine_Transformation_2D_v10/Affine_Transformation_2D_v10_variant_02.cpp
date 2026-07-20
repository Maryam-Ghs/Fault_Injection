#include <iostream>

/* LLM input variant 2: small-diverse */

int main() {
    // version #10
    // 2D affine transformation test (manual loop unrolling, fused expressions, stack arrays)

    // --- deterministic input -------------------------------------------------
    // transformation matrix components
    float a = -0.8f;   // scale‑x
    float b = 1.1f;    // shear‑x
    float tx = 3.5f;   // translate‑x
    float c = 0.4f;    // shear‑y
    float d = -1.3f;   // scale‑y
    float ty = 2.0f;   // translate‑y

    // points to transform (x, y)
    const int N = 5; // fixed size, known at compile time
    float px[N] = { 2.0f, -4.5f, 0.0f, 1.2f, -3.3f };
    float py[N] = { -1.0f, 3.3f, -2.2f, 0.0f, 4.4f };

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
