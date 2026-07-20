#include <iostream>
#include <cmath>
#include <iomanip>

/* LLM input variant 2: small-diverse */

int main() {
    // version #9 – Angle at Vertex (loop‑heavy, stack arrays, fused expressions)

    // deterministic test vectors: each row = {x0, y0, x1, y1, x2, y2}
    float data[5][6] = {
        {1.0f, 0.0f, 0.0f, 0.0f, 0.8660254f, 0.5f},   // 30°
        {1.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.8660254f},   // 60°
        {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f},         // 90°
        {1.0f, 0.0f, 0.0f, 0.0f, -0.5f, 0.8660254f},  // 120°
        {1.0f, 0.0f, 0.0f, 0.0f, -0.8660254f, 0.5f}   // 150°
    };

    const int cases = 5;          // number of test triples
    const int dims  = 2;          // 2‑D points

    std::cout << std::fixed << std::setprecision(2);

    for (int idx = 0; idx < cases; ++idx) {
        // build the two edge vectors around the middle point (vertex)
        float vecA[2];
        float vecB[2];
        vecA[0] = data[idx][0] - data[idx][2];  // x0 - x1
        vecA[1] = data[idx][1] - data[idx][3];  // y0 - y1
        vecB[0] = data[idx][4] - data[idx][2];  // x2 - x1
        vecB[1] = data[idx][5] - data[idx][3];  // y2 - y1

        // dot product and squared lengths using a single loop (fused)
        float dotAB = 0.0f;
        float lenSqA = 0.0f;
        float lenSqB = 0.0f;
        for (int d = 0; d < dims; ++d) {
            dotAB   += vecA[d] * vecB[d];
            lenSqA  += vecA[d] * vecA[d];
            lenSqB  += vecB[d] * vecB[d];
        }

        // magnitude product (reordered: sqrt after multiplication)
        float magProd = sqrtf(lenSqA * lenSqB);

        // angle in radians, then convert to degrees (fused expression)
        float angleDeg = acosf(dotAB / magProd) * (180.0f / 3.14159265f);

        std::cout << "Case " << idx + 1 << ": " << angleDeg << " degrees\n";
    }
    return 0;
}
