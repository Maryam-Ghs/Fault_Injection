/* LLM input variant 9: medium-deterministic-random */
// Angle at Vertex – version #8
// Implements a loop‑heavy, heap‑based solution with helper functions.
// Uses only int and float (no double, long, unsigned, const).

#include <iostream>
#include <cmath>

// Simple 2‑D point
struct Pt {
    float x;
    float y;
};

// Deterministic pseudo‑random generator (linear congruential)
static int lcg_state = 123456789;
int lcg_next() {
    lcg_state = (lcg_state * 1103515245 + 12345) & 0x7fffffff;
    return lcg_state;
}

// Convert LCG output to a float in range [-10.0, 10.0]
float lcg_float() {
    int v = lcg_next() % 2001;               // 0 .. 2000
    return (static_cast<float>(v) - 1000.0f) / 100.0f; // -10.00 .. 10.00
}

// Allocate a flat array of points for all test cases on the heap
Pt* allocateData(int caseCount) {
    // each case stores three points (A, V, B)
    Pt* data = new Pt[caseCount * 3];
    for (int i = 0; i < caseCount * 3; ++i) {
        data[i].x = lcg_float();
        data[i].y = lcg_float();
    }
    return data;
}

// Compute the angle (in degrees) at the middle point of three points
float angleAtVertex(const Pt& a, const Pt& v, const Pt& b) {
    // vector VA = A - V
    float vx1 = a.x - v.x;
    float vy1 = a.y - v.y;
    // vector VB = B - V
    float vx2 = b.x - v.x;
    float vy2 = b.y - v.y;

    // dot product
    float dot = vx1 * vx2 + vy1 * vy2;

    // squared lengths
    float lenSq1 = vx1 * vx1 + vy1 * vy1;
    float lenSq2 = vx2 * vx2 + vy2 * vy2;

    // lengths (sqrt)
    float len1 = sqrtf(lenSq1);
    float len2 = sqrtf(lenSq2);

    // product of lengths
    float prodLen = len1 * len2;

    // protect against division by zero
    if (prodLen == 0.0f) return 0.0f;

    // cosine of the angle
    float cosTheta = dot / prodLen;

    // clamp to valid range [-1,1] to avoid NaN from acosf
    if (cosTheta > 1.0f) cosTheta = 1.0f;
    if (cosTheta < -1.0f) cosTheta = -1.0f;

    // angle in radians, then to degrees
    float rad = acosf(cosTheta);
    float deg = rad * (180.0f / 3.1415926f);
    return deg;
}

// Print the result for each test case
void outputResult(int idx, float angleDeg) {
    std::cout << "Case " << idx << ": angle = " << angleDeg << " degrees" << std::endl;
}

// Main driver
int main() {
    int totalCases = 20;                     // increased number of deterministic cases
    Pt* allPoints = allocateData(totalCases);

    // iterate over cases using a loop‑heavy style
    int caseIdx = 0;
    while (caseIdx < totalCases) {
        // compute offsets for the three points of this case
        int base = caseIdx * 3;

        // retrieve the three points
        Pt pA = allPoints[base];
        Pt pV = allPoints[base + 1];
        Pt pB = allPoints[base + 2];

        // compute the angle
        float angle = angleAtVertex(pA, pV, pB);

        // print the result
        outputResult(caseIdx, angle);

        // advance to next case
        caseIdx = caseIdx + 1;
    }

    // clean up heap memory
    delete[] allPoints;
    return 0;
}
