/* LLM input variant 3: zeros-and-ones */
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
    // Deterministic test vectors (5‑D) emphasizing zeros, ones, and identity
    Vector vA(5);
    vA.put(0, 1.0f);
    vA.put(1, 0.0f);
    vA.put(2, 0.0f);
    vA.put(3, 0.0f);
    vA.put(4, 0.0f);

    Vector vB(5);
    vB.put(0, 1.0f);
    vB.put(1, 0.0f);
    vB.put(2, 0.0f);
    vB.put(3, 0.0f);
    vB.put(4, 0.0f);

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
