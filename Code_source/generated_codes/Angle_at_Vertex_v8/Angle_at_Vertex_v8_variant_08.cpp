/* LLM input variant 8: sparse-skewed */
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

// Allocate a flat array of points for all test cases on the heap
Pt* allocateData(int caseCount) {
    // each case stores three points (A, V, B)
    Pt* data = new Pt[caseCount * 3];

    // case 0: basic right triangle
    data[0].x = 0.0f;   data[0].y = 0.0f;   // A
    data[1].x = 1.0f;   data[1].y = 0.0f;   // V
    data[2].x = 1.0f;   data[2].y = 1.0f;   // B

    // case 1: extremely far points (sparse, skewed)
    data[3].x = -1e6f;  data[3].y = -1e6f;
    data[4].x = 0.0f;   data[4].y = 0.0f;
    data[5].x = 1e6f;   data[5].y = -1e6f;

    // case 2: V coincides with A (zero‑length vector VA)
    data[6].x = 2.0f;   data[6].y = 2.0f;   // A
    data[7].x = 2.0f;   data[7].y = 2.0f;   // V
    data[8].x = 3.0f;   data[8].y = 3.0f;   // B

    // case 3: V coincides with B (zero‑length vector VB)
    data[9].x = -3.0f;  data[9].y = -3.0f;  // A
    data[10].x = -2.0f; data[10].y = -2.0f; // V
    data[11].x = -2.0f; data[11].y = -2.0f; // B

    // case 4: clustered near origin (dense cluster)
    data[12].x = 0.001f; data[12].y = 0.002f;
    data[13].x = 0.0015f;data[13].y = 0.001f;
    data[14].x = 0.002f; data[14].y = 0.003f;

    // case 5: one extreme point, others near zero (high skew)
    data[15].x = 0.0f;    data[15].y = 0.0f;   // A
    data[16].x = 0.0f;    data[16].y = 0.0f;   // V
    data[17].x = 1e5f;    data[17].y = 1e5f;   // B

    // case 6: almost colinear points (small angle)
    data[18].x = 0.0f;    data[18].y = 0.0f;   // A
    data[19].x = 1.0f;    data[19].y = 1.0f;   // V
    data[20].x = 2.0f;    data[20].y = 2.001f; // B

    // case 7: large empty space between points
    data[21].x = -5000.0f; data[21].y = 0.0f;
    data[22].x = 0.0f;    data[22].y = 0.0f;
    data[23].x = 5000.0f; data[23].y = 0.0f;

    // case 8: points forming a very obtuse angle
    data[24].x = -1.0f;   data[24].y = 0.0f;
    data[25].x = 0.0f;    data[25].y = 0.0f;
    data[26].x = 0.5f;    data[26].y = -0.1f;

    // case 9: degenerate case with all points identical
    data[27].x = 5.0f;    data[27].y = 5.0f;
    data[28].x = 5.0f;    data[28].y = 5.0f;
    data[29].x = 5.0f;    data[29].y = 5.0f;

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
    int totalCases = 10;                     // number of predefined cases (sparse‑skewed)
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
