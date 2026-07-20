#include <bits/stdc++.h>
using namespace std;

/* LLM input variant 9: medium-deterministic-random */

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
    const int CASES = 12;
    int data[CASES][3][2];

    // deterministic pseudo‑random generation (LCG)
    long long seed = 123456789LL;
    const long long A = 6364136223846793005LL;
    const long long C = 1LL;
    const long long MOD = 1LL << 31; // 2^31
    for (int i = 0; i < CASES; ++i) {
        for (int p = 0; p < 3; ++p) {
            // generate x
            seed = (A * seed + C) % MOD;
            int x = (int)(seed % 1001) - 500; // range [-500,500]
            // generate y
            seed = (A * seed + C) % MOD;
            int y = (int)(seed % 1001) - 500;
            data[i][p][0] = x;
            data[i][p][1] = y;
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
