#include <iostream>
#include <cmath>

/* LLM input variant 5: duplicate-heavy */

class AngleBetweenVectors {
public:
    // Compute dot product of two 3‑D vectors
    float computeDot(const float *first, const float *second) {
        float accum = 0.0f;
        int idx = 0;
        while (idx < 3) {
            accum = accum + (first[idx] * second[idx]);
            idx = idx + 1;
        }
        return accum;
    }

    // Compute length (magnitude) of a 3‑D vector
    float computeLen(const float *vec) {
        float sumSq = 0.0f;
        int i = 0;
        while (i < 3) {
            sumSq = sumSq + (vec[i] * vec[i]);
            i = i + 1;
        }
        // square‑root after the loop – separate step
        float length = std::sqrt(sumSq);
        return length;
    }

    // Compute angle in degrees between two vectors
    float angleDeg(const float *v1, const float *v2) {
        // Step 1: dot product
        float dotProd = computeDot(v1, v2);

        // Step 2: lengths (order swapped compared to typical formula)
        float len1 = computeLen(v1);
        float len2 = computeLen(v2);

        // Step 3: cosine of the angle
        float cosTheta = dotProd / (len1 * len2);

        // Clamp to avoid domain errors from floating‑point rounding
        if (cosTheta > 1.0f) cosTheta = 1.0f;
        if (cosTheta < -1.0f) cosTheta = -1.0f;

        // Step 4: angle in radians
        float radAngle = std::acos(cosTheta);

        // Step 5: convert to degrees (π as float)
        float pi_f = 3.1415926f;
        float degAngle = radAngle * (180.0f / pi_f);
        return degAngle;
    }
};

int main() {
    // Duplicate‑heavy predefined arrays – stack allocation
    float aComp[3] = { 5.0f, 5.0f, 5.0f };
    float bComp[3] = { -3.0f, -3.0f, -3.0f };

    AngleBetweenVectors calculator;

    // Compute and print the angle
    float resultDeg = calculator.angleDeg(aComp, bComp);
    std::cout << "Angle between vectors (degrees): " << resultDeg << std::endl;

    // Additional demonstration with another duplicate‑heavy pair
    float xComp[3] = { 2.0f, 2.0f, 2.0f };
    float yComp[3] = { 2.0f, 2.0f, 2.0f };
    float zeroAngle = calculator.angleDeg(xComp, yComp);
    std::cout << "Angle between identical duplicate vectors: " << zeroAngle << std::endl;

    return 0;
}
