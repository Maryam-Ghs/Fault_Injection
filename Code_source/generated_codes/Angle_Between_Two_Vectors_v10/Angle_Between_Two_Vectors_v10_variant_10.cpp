#include <iostream>
#include <cmath>
#include <vector>

/* LLM input variant 10: large-safe-stress */

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
    // Large deterministic test: 5,000 pairs of 3‑D vectors
    const int pairCount = 5000;
    std::vector<float> aData(pairCount * 3);
    std::vector<float> bData(pairCount * 3);

    for (int i = 0; i < pairCount; ++i) {
        aData[i * 3 + 0] = static_cast<float>((i % 100) - 50);
        aData[i * 3 + 1] = static_cast<float>(((i * 2) % 100) - 50);
        aData[i * 3 + 2] = static_cast<float>(((i * 3) % 100) - 50);

        bData[i * 3 + 0] = static_cast<float>(((i * 5) % 100) - 50);
        bData[i * 3 + 1] = static_cast<float>(((i * 7) % 100) - 50);
        bData[i * 3 + 2] = static_cast<float>(((i * 11) % 100) - 50);
    }

    AngleBetweenVectors calculator;

    // Compute and display angle for the first and last pair to keep output concise
    float firstAngle = calculator.angleDeg(&aData[0], &bData[0]);
    std::cout << "First large test angle (degrees): " << firstAngle << std::endl;

    float lastAngle = calculator.angleDeg(&aData[(pairCount - 1) * 3],
                                          &bData[(pairCount - 1) * 3]);
    std::cout << "Last large test angle (degrees): " << lastAngle << std::endl;

    // Original small predefined arrays – stack allocation (kept for sanity check)
    float aComp[3] = { 3.0f, -2.0f, 5.0f };
    float bComp[3] = { -1.0f, 4.0f, 2.0f };

    // Compute and print the angle for the original example
    float resultDeg = calculator.angleDeg(aComp, bComp);
    std::cout << "Angle between vectors (degrees): " << resultDeg << std::endl;

    // Additional demonstration with another pair
    float xComp[3] = { 1.0f, 0.0f, 0.0f };
    float yComp[3] = { 0.0f, 1.0f, 0.0f };
    float rightAngle = calculator.angleDeg(xComp, yComp);
    std::cout << "Angle between orthogonal unit vectors: " << rightAngle << std::endl;

    return 0;
}
