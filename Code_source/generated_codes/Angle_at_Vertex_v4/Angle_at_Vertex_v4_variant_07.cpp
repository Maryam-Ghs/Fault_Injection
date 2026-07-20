#include <bits/stdc++.h>
using namespace std;

/* LLM input variant 7: reverse-adversarial */

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
    // reversed and adversarial dataset (stack array)
    int data[12][3][2] = {
        {{0,0},{0,0},{0,0}},                     // all zero (original case 12)
        {{0,0},{0,-5},{0,5}},                    // straight line reversed (case 11)
        {{0,0},{-3,4},{3,4}},                    // acute reversed (case 10)
        {{0,0},{-1,-1},{-1,0}},                  // 45° reversed (case 9)
        {{0,0},{1,1},{1,0}},                     // 45° reversed (case 8)
        {{0,0},{2,0},{1,0}},                     // collinear (0°) reversed (case 7)
        {{-1000000,-1000000},{-1000000,1000000},{1000000,1000000}}, // large coords reversed (case 6, scaled)
        {{5,5},{5,5},{5,5}},                     // all identical (case 5)
        {{0,0},{2,2},{1,1}},                     // collinear (180°) reversed (case 4)
        {{0,0},{1,1},{0,0}},                     // zero‑length first edge reversed (case 3)
        {{0,0},{0,-1},{-1,0}},                   // 90° reversed (case 2)
        {{0,0},{0,1},{1,0}}                      // 90° reversed (case 1)
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
