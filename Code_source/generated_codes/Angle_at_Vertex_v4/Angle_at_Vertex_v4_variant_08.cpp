#include <bits/stdc++.h>
using namespace std;

/* LLM input variant 8: sparse-skewed */

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
    // Sparse‑skewed dataset (mostly zeros, few extreme points)
    int data[12][3][2] = {
        {{0,0},{0,0},{0,0}},                         // all zero
        {{0,0},{0,0},{1,0}},                         // single unit edge
        {{0,0},{0,0},{0,0}},                         // duplicate zero case
        {{0,0},{0,0},{0,0}},                         // another zero case
        {{0,0},{1000000,0},{0,1000000}},             // large sparse coordinates
        {{0,0},{-1000000,-1000000},{1000000,1000000}}, // opposite extremes
        {{0,0},{1,0},{0,1}},                         // standard right angle
        {{0,0},{1,0},{2,0}},                         // collinear (0°)
        {{0,0},{0,0},{0,0}},                         // zero again
        {{0,0},{0,5},{0,-5}},                        // straight line (180°)
        {{0,0},{3,4},{-3,4}},                        // acute angle
        {{0,0},{-1,0},{-1,-1}}                       // 45° angle
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
