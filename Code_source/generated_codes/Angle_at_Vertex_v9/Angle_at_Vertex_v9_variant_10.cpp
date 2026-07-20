#include <iostream>
#include <cmath>
#include <iomanip>

/* LLM input variant 10: large-safe-stress */

int main() {
    // version #9 – Angle at Vertex (loop‑heavy, stack arrays, fused expressions)

    const int cases = 1000;          // larger number of test triples (safe for stack)
    const int dims  = 2;              // 2‑D points
    const float PI = 3.14159265358979323846f;

    // deterministic test vectors: each row = {x0, y0, x1, y1, x2, y2}
    // middle vertex (x1,y1) is set to the origin (0,0) for all cases.
    // points are placed on the unit circle at angles that vary with the case index.
    float data[cases][6];
    for (int i = 0; i < cases; ++i) {
        float angle0 = static_cast<float>((i % 360)) * PI / 180.0f;          // angle for point 0
        float angle2 = static_cast<float>(((i * 2) % 360)) * PI / 180.0f;  // angle for point 2

        data[i][0] = std::cos(angle0);  // x0
        data[i][1] = std::sin(angle0);  // y0
        data[i][2] = 0.0f;              // x1 (vertex)
        data[i][3] = 0.0f;              // y1 (vertex)
        data[i][4] = std::cos(angle2); // x2
        data[i][5] = std::sin(angle2); // y2
    }

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
        float angleDeg = acosf(dotAB / magProd) * (180.0f / PI);

        std::cout << "Case " << idx + 1 << ": " << angleDeg << " degrees\n";
    }
    return 0;
}
