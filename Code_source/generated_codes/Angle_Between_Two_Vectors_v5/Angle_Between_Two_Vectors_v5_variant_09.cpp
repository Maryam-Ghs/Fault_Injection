/* LLM input variant 9: medium-deterministic-random */
// Angle Between Two Vectors – Version 5 (variant 9)
#include <iostream>
#include <cmath>

class Vector {
public:
    int dim;          // number of components
    float* data;      // heap‑allocated array

    // Constructor: allocate array
    Vector(int d) {
        dim = d;
        data = new float[dim];
    }

    // Destructor: free array
    ~Vector() {
        delete[] data;
    }

    // Fill component at position idx
    void put(int idx, float val) {
        if (idx >= 0 && idx < dim) {
            data[idx] = val;
        }
    }

    // Dot product with another vector
    float dot(Vector& other) {
        float sum = 0.0f;
        int i = 0;
        while (i < dim) {
            sum += data[i] * other.data[i];
            ++i;
        }
        return sum;
    }

    // Euclidean norm (length)
    float length() {
        float acc = 0.0f;
        int i = 0;
        while (i < dim) {
            acc += data[i] * data[i];
            ++i;
        }
        return sqrtf(acc);
    }
};

// Simple deterministic pseudo‑random generator (LCG)
static unsigned int lcg_state = 12345u;
unsigned int lcg_next() {
    lcg_state = (1103515245u * lcg_state + 12345u) & 0x7fffffff;
    return lcg_state;
}

// Fill vector with deterministic pseudo‑random floats in range [-10, 10]
void fill_vector(Vector& v) {
    for (int i = 0; i < v.dim; ++i) {
        // Generate integer in [0, 0x7fffffff], then map to [-10,10]
        unsigned int rnd = lcg_next();
        float val = (static_cast<float>(rnd) / 0x7fffffff) * 20.0f - 10.0f;
        v.put(i, val);
    }
}

int main() {
    // Deterministic test vectors (10‑D)
    const int DIM = 10;
    Vector vA(DIM);
    Vector vB(DIM);

    fill_vector(vA);
    fill_vector(vB);

    // Compute dot product and magnitudes
    float dp = vA.dot(vB);
    float lenA = vA.length();
    float lenB = vB.length();

    // Guard against zero-length vectors (should not happen with our generator)
    if (lenA == 0.0f || lenB == 0.0f) {
        std::cout << "One of the vectors has zero length; cannot compute angle." << std::endl;
        return 1;
    }

    // Compute cosine of the angle, clamp to [-1,1] for safety
    float cosTheta = dp / (lenA * lenB);
    if (cosTheta > 1.0f) cosTheta = 1.0f;
    if (cosTheta < -1.0f) cosTheta = -1.0f;

    // Angle in radians then convert to degrees
    float rad = acosf(cosTheta);
    float deg = rad * 180.0f / 3.14159265f;

    // Output
    std::cout << "Angle between vectors (degrees): " << deg << std::endl;

    return 0;
}
