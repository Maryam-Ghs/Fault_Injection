// Angle at Vertex – version #8
/* LLM input variant 4: signed-extremes */

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
    // small predefined test set with mixed signed values

    // case 0: simple right triangle
    data[0].x = 0.0f;  data[0].y = 0.0f;   // A
    data[1].x = 1.0f;  data[1].y = 0.0f;   // V
    data[2].x = 1.0f;  data[2].y = 1.0f;   // B

    // case 1: symmetric across origin, includes negatives
    data[3].x = -2.0f; data[3].y = -2.0f;
    data[4].x = 0.0f;  data[4].y = 0.0f;
    data[5].x = 2.0f;  data[5].y = -2.0f;

    // case 2: V coincides with A (zero‑length side)
    data[6].x = 3.0f; data[6].y = 3.0f;   // A
    data[7].x = 3.0f; data[7].y = 3.0f;   // V (same as A)
    data[8].x = 6.0f; data[8].y = 3.0f;   // B

    // case 3: large positive coordinates
    data[9].x  = 10000.0f;  data[9].y  = 10000.0f;
    data[10].x = 15000.0f;  data[10].y = 10000.0f;
    data[11].x = 10000.0f;  data[11].y = 15000.0f;

    // case 4: mix of zero and negative values
    data[12].x = 0.0f;   data[12].y = -5.0f;
    data[13].x = -5.0f;  data[13].y = 0.0f;
    data[14].x = 0.0f;   data[14].y = 5.0f;

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
    int totalCases = 5;                     // number of predefined cases
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
