/* LLM input variant 7: reverse-adversarial */
#include <iostream>
#include <cmath>
#include <iomanip>

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
    // Reverse‑ordered, adversarial test set (stored on the heap)
    int caseCount = 7;
    float** data = new float*[caseCount];

    // 1. Near‑colinear huge coordinates (angle ~0°)
    data[0] = new float[6]{1000000.0f, 1000000.0f,
                           2000000.0f, 2000000.0f,
                           3000000.0f, 3000001.0f};

    // 2. Large right angle (90°) with reversed point order
    data[1] = new float[6]{0.0f, 1000000.0f,
                           0.0f, 0.0f,
                           1000000.0f, 0.0f};

    // 3. Straight line reversed (180°)
    data[2] = new float[6]{0.0f, 0.0f,
                           1.0f, 0.0f,
                           2.0f, 0.0f};

    // 4. Duplicate middle point with large values (zero‑length vectors)
    data[3] = new float[6]{500000.0f, 500000.0f,
                           0.0f, 0.0f,
                           500000.0f, 500000.0f};

    // 5. Obtuse angle with large spread
    data[4] = new float[6]{-2000.0f, -2000.0f,
                           0.0f, 0.0f,
                           2000.0f, 1000.0f};

    // 6. Acute angle with modest coordinates (reverse order)
    data[5] = new float[6]{2.0f, 1.0f,
                           0.0f, 0.0f,
                           1.0f, 0.0f};

    // 7. Normal triangle but points given in reverse clockwise order
    data[6] = new float[6]{1.0f, 1.0f,
                           0.0f, 0.0f,
                           0.0f, 1.0f};

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
