/* LLM input variant 5: duplicate-heavy */

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

    // Fill vector with a duplicate‑heavy deterministic pattern
    static void duplicateFill(float* vec, int sz) {
        // Use a repeating pattern of values: 5, -5, 5, -5, ...
        for (int i = 0; i < sz; ++i) {
            vec[i] = (i % 2 == 0) ? 5.0f : -5.0f;
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
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    const int dim = 7;               // medium‑size duplicate‑heavy vectors
    float* vecX = VectorMath::allocate(dim);
    float* vecY = VectorMath::allocate(dim);

    VectorMath::duplicateFill(vecX, dim);
    VectorMath::duplicateFill(vecY, dim);

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
