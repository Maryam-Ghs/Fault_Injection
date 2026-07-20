#include <bits/stdc++.h>
using namespace std;

/* LLM input variant 3: zeros-and-ones */

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
    // zero‑and‑one heavy dataset (stack array)
    int data[12][3][2] = {
        {{0,0},{1,0},{0,1}},                 // 90°
        {{0,0},{1,0},{1,1}},                 // 45°
        {{0,0},{0,0},{0,0}},                 // all zero
        {{0,0},{1,0},{1,0}},                 // zero‑length second edge
        {{0,0},{0,1},{0,1}},                 // zero‑length second edge
        {{0,0},{1,1},{0,0}},                 // duplicate point0 and point2
        {{0,0},{1,0},{0,0}},                 // zero‑length second edge
        {{0,0},{0,1},{1,1}},                 // 45°
        {{0,0},{1,0},{1,1}},                 // duplicate 45°
        {{1,0},{1,0},{1,0}},                 // all ones
        {{0,0},{1,0},{0,1}},                 // duplicate 90°
        {{0,0},{0,0},{1,1}}                  // zero‑length first edge
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
