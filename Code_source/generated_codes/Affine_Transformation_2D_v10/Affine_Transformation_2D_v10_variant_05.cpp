#include <iostream>

/* LLM input variant 5: duplicate-heavy */

int main() {
    // version #10
    // 2D affine transformation test (manual loop unrolling, fused expressions, stack arrays)

    // --- deterministic input -------------------------------------------------
    // transformation matrix components (duplicate values)
    float a = 1.0f;   // scale‑x
    float b = 1.0f;   // shear‑x (duplicate of a)
    float tx = 5.0f;  // translate‑x
    float c = 1.0f;   // shear‑y (duplicate of a)
    float d = 1.0f;   // scale‑y (duplicate of a)
    float ty = 5.0f;  // translate‑y (duplicate of tx)

    // points to transform (x, y) with repeated patterns
    const int N = 6; // fixed size, known at compile time
    float px[N] = { 2.0f, 2.0f, -1.0f, -1.0f, 2.0f, -1.0f };
    float py[N] = { 3.0f, 3.0f, 3.0f, 3.0f, 3.0f, 3.0f };

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
