#include <bits/stdc++.h>
using namespace std;

/* LLM input variant 6: ordered-structured */

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
    // ordered, structured dataset (stack array)
    int data[12][3][2] = {
        {{0,0},{1,0},{0,1}},          // case 1: right angle
        {{1,1},{2,2},{3,3}},          // case 2: collinear
        {{2,2},{3,2},{2,3}},          // case 3: right angle
        {{3,3},{4,4},{5,5}},          // case 4: collinear
        {{4,4},{5,4},{4,5}},          // case 5: right angle
        {{5,5},{6,6},{7,7}},          // case 6: collinear
        {{6,6},{7,6},{6,7}},          // case 7: right angle
        {{7,7},{8,8},{9,9}},          // case 8: collinear
        {{8,8},{9,8},{8,9}},          // case 9: right angle
        {{9,9},{10,10},{11,11}},      // case10: collinear
        {{10,10},{11,10},{10,11}},    // case11: right angle
        {{11,11},{12,12},{13,13}}     // case12: collinear
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
