#include <bits/stdc++.h>
using namespace std;

/* LLM input variant 10: large-safe-stress */

class VertexAngle {
public:
    // points[3][2] holds three (x,y) integer pairs
    float evaluate(int points[3][2]) {
        // vector A -> B and A -> C
        float ax = (float)(points[1][0] - points[0][0]);
        float ay = (float)(points[1][1] - points[0][1]);
        float bx = (float)(points[2][0] - points[0][0]);
        float by = (float)(points[2][1] - points[0][1]);

        // fused dot product and lengths
        float dot = ax * bx + ay * by;
        float lenA = sqrtf(ax * ax + ay * ay);
        float lenB = sqrtf(bx * bx + by * by);

        // guard against zero‑length edges
        if (lenA == 0.0f || lenB == 0.0f) return 0.0f;

        // fused cosine computation
        float cosTheta = dot / (lenA * lenB);
        if (cosTheta > 1.0f) cosTheta = 1.0f;
        if (cosTheta < -1.0f) cosTheta = -1.0f;

        // angle in degrees (float only)
        return acosf(cosTheta) * 180.0f / 3.14159265f;
    }
};

int main() {
    const int CASES = 1000;                 // large but safe number of test cases
    int data[CASES][3][2];                  // stack allocation stays within reasonable limits

    // deterministic generation of a diverse, stress‑testing dataset
    for (int i = 0; i < CASES; ++i) {
        // Base coordinate grows with i to test large values
        int base = i * 1000;

        // Default points forming a generic triangle
        data[i][0][0] = base;
        data[i][0][1] = base;
        data[i][1][0] = base + 12345;
        data[i][1][1] = base - 6789;
        data[i][2][0] = base - 5432;
        data[i][2][1] = base + 9876;

        // Insert special edge cases periodically
        if (i % 100 == 0) {
            // Zero‑length first edge
            data[i][1][0] = data[i][0][0];
            data[i][1][1] = data[i][0][1];
        } else if (i % 100 == 1) {
            // Collinear points (straight line)
            data[i][2][0] = data[i][1][0] * 2 - data[i][0][0];
            data[i][2][1] = data[i][1][1] * 2 - data[i][0][1];
        } else if (i % 100 == 2) {
            // All identical points
            data[i][1][0] = data[i][0][0];
            data[i][1][1] = data[i][0][1];
            data[i][2][0] = data[i][0][0];
            data[i][2][1] = data[i][0][1];
        } else if (i % 100 == 3) {
            // Very large coordinates near int limits
            data[i][0][0] = INT_MAX - 1;
            data[i][0][1] = INT_MAX - 2;
            data[i][1][0] = INT_MAX - 3;
            data[i][1][1] = INT_MAX - 4;
            data[i][2][0] = INT_MAX - 5;
            data[i][2][1] = INT_MAX - 6;
        } else if (i % 100 == 4) {
            // Small triangle producing a 45° angle
            data[i][0][0] = 0; data[i][0][1] = 0;
            data[i][1][0] = 1; data[i][1][1] = 0;
            data[i][2][0] = 1; data[i][2][1] = 1;
        }
    }

    VertexAngle calc;

    // loop‑heavy iterative processing
    for (int i = 0; i < CASES; ++i) {
        int (*pts)[2] = data[i];
        float angleDeg = calc.evaluate(pts);
        printf("Case %d: %.2f degrees\n", i + 1, angleDeg);
    }
    return 0;
}
