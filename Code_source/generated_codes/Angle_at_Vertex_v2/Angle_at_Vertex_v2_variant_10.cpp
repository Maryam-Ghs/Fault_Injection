#include <iostream>
#include <cmath>
#include <iomanip>

/* LLM input variant 10: large-safe-stress */

class VertexAngle {
public:
    // Compute the angle (in degrees) at the middle point of three points stored in heap.
    float evaluate(float* coords) {               // coords = [x0,y0, x1,y1, x2,y2]
        // Vectors: v0 = P0 - P1, v2 = P2 - P1
        float vx0 = coords[0] - coords[2];
        float vy0 = coords[1] - coords[3];
        float vx2 = coords[4] - coords[2];
        float vy2 = coords[5] - coords[3];

        // Dot product and squared lengths using loop‑heavy fused operations
        float dot = 0.0f;
        float lenSq0 = 0.0f;
        float lenSq2 = 0.0f;
        for (int i = 0; i < 2; ++i) {
            float a = (i == 0) ? vx0 : vy0;
            float b = (i == 0) ? vx2 : vy2;
            dot += a * b;                         // fused: multiplication then addition
            lenSq0 += a * a;                      // fused for first vector length
            lenSq2 += b * b;                      // fused for second vector length
        }

        // Guard against zero‑length vectors
        if (lenSq0 == 0.0f || lenSq2 == 0.0f) {
            return 0.0f;                         // undefined angle, report as 0
        }

        // Compute cosine, clamp to [-1,1] to avoid domain errors
        float cosTheta = dot / (std::sqrt(lenSq0) * std::sqrt(lenSq2));
        if (cosTheta > 1.0f) cosTheta = 1.0f;
        if (cosTheta < -1.0f) cosTheta = -1.0f;

        // Angle in radians then convert to degrees
        float rad = std::acos(cosTheta);
        return rad * (180.0f / 3.14159265358979323846f);
    }
};

int main() {
    // Large but safe stress test set (stored on the heap)
    const int caseCount = 1000;
    float** data = new float*[caseCount];

    for (int i = 0; i < caseCount; ++i) {
        data[i] = new float[6];
        // Generate diverse patterns based on i mod 5
        switch (i % 5) {
            case 0: { // Normal triangle, scaled with i
                float base = static_cast<float>(i);
                data[i][0] = base;          data[i][1] = 0.0f;
                data[i][2] = base + 1.0f;   data[i][3] = 0.0f;
                data[i][4] = base + 1.0f;   data[i][5] = 1.0f;
                break;
            }
            case 1: { // Colinear points on line y = x
                float base = static_cast<float>(i) * 0.1f;
                data[i][0] = base;          data[i][1] = base;
                data[i][2] = base + 1.0f;    data[i][3] = base + 1.0f;
                data[i][4] = base + 2.0f;    data[i][5] = base + 2.0f;
                break;
            }
            case 2: { // Straight line (180°) horizontal
                float base = static_cast<float>(i) * 0.2f;
                data[i][0] = base + 2.0f;    data[i][1] = 0.0f;
                data[i][2] = base + 1.0f;    data[i][3] = 0.0f;
                data[i][4] = base;          data[i][5] = 0.0f;
                break;
            }
            case 3: { // Duplicate middle point (zero‑length vector)
                float base = static_cast<float>(i) * 0.3f;
                data[i][0] = base;          data[i][1] = base + 1.0f;
                data[i][2] = base;          data[i][3] = base;          // middle point duplicate
                data[i][4] = base + 1.0f;    data[i][5] = base;
                break;
            }
            case 4: { // Alternating acute/obtuse angles
                float base = static_cast<float>(i);
                if ((i / 5) % 2 == 0) { // acute
                    data[i][0] = base;          data[i][1] = 0.0f;
                    data[i][2] = base + 1.0f;   data[i][3] = 0.0f;
                    data[i][4] = base + 2.0f;   data[i][5] = 0.5f;
                } else { // obtuse
                    data[i][0] = base;          data[i][1] = 0.0f;
                    data[i][2] = base + 1.0f;   data[i][3] = 0.0f;
                    data[i][4] = base + 2.0f;   data[i][5] = 3.0f;
                }
                break;
            }
        }
    }

    VertexAngle calculator;

    std::cout << std::fixed << std::setprecision(2);
    for (int idx = 0; idx < caseCount; ++idx) {
        float result = calculator.evaluate(data[idx]);
        std::cout << "Case " << idx + 1 << ": " << result << " degrees\n";
    }

    // Clean up heap memory
    for (int i = 0; i < caseCount; ++i) {
        delete[] data[i];
    }
    delete[] data;

    return 0;
}
