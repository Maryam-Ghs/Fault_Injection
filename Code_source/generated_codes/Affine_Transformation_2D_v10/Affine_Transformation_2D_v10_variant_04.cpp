#include <iostream>

/* LLM input variant 4: signed-extremes */

int main() {
    // version #10
    // 2D affine transformation test (manual loop unrolling, fused expressions, stack arrays)

    // --- deterministic input -------------------------------------------------
    // transformation matrix components
    float a = -2.5f;   // scale‑x (negative)
    float b = 0.0f;    // shear‑x (zero)
    float tx = 0.0f;   // translate‑x (zero)
    float c = 3.3f;    // shear‑y (positive)
    float d = -1.0f;   // scale‑y (negative)
    float ty = 15.0f;  // translate‑y (positive)

    // points to transform (x, y)
    const int N = 6; // fixed size, known at compile time
    float px[N] = { -1000.0f, 0.0f, 500.0f, -250.5f, 0.0f, 123.456f };
    float py[N] = { 0.0f, -500.0f, 0.0f, 250.25f, -123.456f, 0.0f };

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
    // handle possible odd element (none for N=6, but keep for completeness)
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
