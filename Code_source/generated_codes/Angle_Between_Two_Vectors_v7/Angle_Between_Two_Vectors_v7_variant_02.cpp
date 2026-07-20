/* LLM input variant 2: small-diverse */
// Version #7 – Angle Between Two Vectors (heap, class, loop‑heavy, temp variables)

#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>

class VectorMath {
public:
    // Allocate a vector of given size on the heap
    static float* allocate(int sz) {
        float* ptr = new float[sz];
        return ptr;
    }

    // Fill vector with deterministic test values
    static void deterministicFill(float* vec, int sz, const float* src) {
        for (int i = 0; i < sz; ++i) {
            vec[i] = src[i];
        }
    }

    // Compute dot product using a temporary accumulator
    static float dot(const float* a, const float* b, int sz) {
        float acc = 0.0f;
        for (int i = 0; i < sz; ++i) {
            float term = a[i] * b[i];
            acc += term;
        }
        return acc;
    }

    // Compute Euclidean norm (length) – split into squared sum and sqrt
    static float norm(const float* v, int sz) {
        float sqSum = 0.0f;
        for (int i = 0; i < sz; ++i) {
            float sq = v[i] * v[i];
            sqSum += sq;
        }
        float length = std::sqrt(sqSum);
        return length;
    }

    // Compute angle (in radians) between two vectors
    static float angle(const float* a, const float* b, int sz) {
        float dp   = dot(a, b, sz);          // dot product
        float lenA = norm(a, sz);            // |a|
        float lenB = norm(b, sz);            // |b|

        // Guard against division by zero
        if (lenA == 0.0f || lenB == 0.0f) {
            return 0.0f;
        }

        float cosTheta = dp / (lenA * lenB);

        // Clamp to [-1,1] to avoid domain errors due to rounding
        if (cosTheta > 1.0f)  cosTheta = 1.0f;
        if (cosTheta < -1.0f) cosTheta = -1.0f;

        float theta = std::acos(cosTheta);  // result is float‑compatible
        return theta;
    }

    // Release heap memory
    static void release(float* ptr) {
        delete[] ptr;
    }
};

int main() {
    const int dim = 4;               // small‑size diverse vectors
    float* vecX = VectorMath::allocate(dim);
    float* vecY = VectorMath::allocate(dim);

    // Deterministic test data with mixed signs and a zero
    const float dataX[dim] = { 3.0f, -2.0f, 0.0f, 5.0f };
    const float dataY[dim] = { 1.0f,  4.0f, -3.0f, 2.0f };

    VectorMath::deterministicFill(vecX, dim, dataX);
    VectorMath::deterministicFill(vecY, dim, dataY);

    // Display the generated vectors
    std::cout << "Vector X: ";
    for (int i = 0; i < dim; ++i) std::cout << vecX[i] << ' ';
    std::cout << "\nVector Y: ";
    for (int i = 0; i < dim; ++i) std::cout << vecY[i] << ' ';
    std::cout << '\n';

    float rad = VectorMath::angle(vecX, vecY, dim);
    std::cout << "Angle (radians) between X and Y: " << rad << '\n';

    VectorMath::release(vecX);
    VectorMath::release(vecY);
    return 0;
}
