/* LLM input variant 9: medium-deterministic-random */

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

    // Fill vector with deterministic pseudo‑random values in range [-10,10]
    static void randomFill(float* vec, int sz) {
        unsigned int seed = 123456789u;          // Fixed seed for reproducibility
        const unsigned int a = 1103515245u;
        const unsigned int c = 12345u;
        const unsigned int m = 1u << 31;        // 2^31
        for (int i = 0; i < sz; ++i) {
            seed = (a * seed + c) % m;
            int val = static_cast<int>(seed % 21) - 10; // -10..10
            vec[i] = static_cast<float>(val);
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
    const int dim = 9;               // medium‑size deterministic vectors
    float* vecX = VectorMath::allocate(dim);
    float* vecY = VectorMath::allocate(dim);

    VectorMath::randomFill(vecX, dim);
    VectorMath::randomFill(vecY, dim);

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
