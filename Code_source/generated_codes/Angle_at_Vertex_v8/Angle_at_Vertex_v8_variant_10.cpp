/ * LLM input variant 10: large-safe-stress * /
/* Angle at Vertex – version #8
   Implements a loop‑heavy, heap‑based solution with helper functions.
   Uses only int and float (no double, long, unsigned, const). */

#include <iostream>
#include <cmath>

// Simple 2‑D point
struct Pt {
    float x;
    float y;
};

// Allocate a flat array of points for all test cases on the heap
Pt* allocateData(int caseCount) {
    // each case stores three points (A, V, B)
    Pt* data = new Pt[caseCount * 3];
    // generate a deterministic large but safe test set
    for (int i = 0; i < caseCount; ++i) {
        // base index for this case
        int base = i * 3;
        // Point A: (i, i)
        data[base].x = static_cast<float>(i);
        data[base].y = static_cast<float>(i);
        // Point V: (i + 0.5, i)
        data[base + 1].x = static_cast<float>(i) + 0.5f;
        data[base + 1].y = static_cast<float>(i);
        // Point B: (i + 0.5, i + 0.5)
        data[base + 2].x = static_cast<float>(i) + 0.5f;
        data[base + 2].y = static_cast<float>(i) + 0.5f;
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
    const int totalCases = 1000;               // large safe stress test set
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
