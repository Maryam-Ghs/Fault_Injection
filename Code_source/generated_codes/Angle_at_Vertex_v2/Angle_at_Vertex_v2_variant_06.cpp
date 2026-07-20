#include <iostream>
#include <cmath>
#include <iomanip>

/* LLM input variant 6: ordered-structured */

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
    // Structured deterministic test set (stored on the heap)
    const int caseCount = 7;
    float** data = new float*[caseCount];
    for (int i = 0; i < caseCount; ++i) {
        data[i] = new float[6];
    }

    // 1. Equilateral triangle (ordered vertices)
    data[0][0] = 0.0f;  data[0][1] = 0.0f;
    data[0][2] = 1.0f;  data[0][3] = 0.0f;
    data[0][4] = 0.5f;  data[0][5] = 0.8660254f;

    // 2. Colinear points increasing along the line y = x
    data[1][0] = 0.0f;  data[1][1] = 0.0f;
    data[1][2] = 1.0f;  data[1][3] = 1.0f;
    data[1][4] = 2.0f;  data[1][5] = 2.0f;

    // 3. Straight line (180°) with points in ascending x order
    data[2][0] = 0.0f;  data[2][1] = 0.0f;
    data[2][2] = 1.0f;  data[2][3] = 0.0f;
    data[2][4] = 2.0f;  data[2][5] = 0.0f;

    // 4. Duplicate middle point (zero‑length vector)
    data[3][0] = 1.0f;  data[3][1] = 1.0f;
    data[3][2] = 1.0f;  data[3][3] = 1.0f;
    data[3][4] = 2.0f;  data[3][5] = 2.0f;

    // 5. Right angle (ordered corners of a unit square)
    data[4][0] = 0.0f;  data[4][1] = 0.0f;
    data[4][2] = 0.0f;  data[4][3] = 1.0f;
    data[4][4] = 1.0f;  data[4][5] = 1.0f;

    // 6. Obtuse angle with symmetric outer points
    data[5][0] = -2.0f; data[5][1] = 0.0f;
    data[5][2] = 0.0f;  data[5][3] = 0.0f;
    data[5][4] = 2.0f;  data[5][5] = 1.0f;

    // 7. Acute angle (smoothly increasing x, modest y)
    data[6][0] = 0.0f;  data[6][1] = 0.0f;
    data[6][2] = 1.0f;  data[6][3] = 0.0f;
    data[6][4] = 2.0f;  data[6][5] = 0.5f;

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
