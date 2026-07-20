/* LLM input variant 2: small-diverse */
#include <bits/stdc++.h>
using namespace std;

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
    // small‑diverse dataset (stack array)
    int data[12][3][2] = {
        {{0,0},{2,0},{0,2}},                 // 90°
        {{1,1},{3,1},{1,3}},                 // 90° shifted
        {{5,5},{5,10},{10,5}},               // 90°
        {{-2,-3},{-2,0},{1,-3}},             // 90°
        {{0,0},{-2,0},{0,-2}},               // 90°
        {{100,200},{150,250},{200,200}},     // acute
        {{-5,5},{0,0},{5,5}},                // 180°
        {{0,0},{1,2},{2,4}},                 // collinear (0°)
        {{-3,-3},{-1,-2},{0,0}},             // acute
        {{10,0},{0,10},{-10,0}},             // obtuse
        {{0,0},{0,0},{1,0}},                 // zero‑length first edge
        {{0,0},{0,0},{0,0}}                  // all zero
    };

    VertexAngle calc;

    // loop‑heavy iterative processing
    for (int i = 0; i < 12; ++i) {
        // fuse pointer extraction
        int (*pts)[2] = data[i];
        float angleDeg = calc.evaluate(pts);
        printf("Case %d: %.2f degrees\n", i + 1, angleDeg);
    }
    return 0;
}
