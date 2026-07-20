/* LLM input variant 10: large-safe-stress */
// Angle Between Two Vectors – Version 5
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

int main() {
    // Deterministic large‑scale test vectors (e.g., 10,000‑D)
    const int DIM = 10000;
    Vector vA(DIM);
    Vector vB(DIM);

    // Fill vectors with uniform values (1.0f) to stay within safe numeric limits
    for (int i = 0; i < DIM; ++i) {
        vA.put(i, 1.0f);
        vB.put(i, 1.0f);
    }

    // Compute dot product and magnitudes
    float dp = vA.dot(vB);
    float lenA = vA.length();
    float lenB = vB.length();

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
