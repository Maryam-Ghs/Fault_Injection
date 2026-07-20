/* LLM input variant 8: sparse-skewed */
// Angle Between Two Vectors – Version 5 (sparse-skewed input)
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
        // Initialize all components to zero (sparse representation)
        for (int i = 0; i < dim; ++i) {
            data[i] = 0.0f;
        }
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
    // Deterministic sparse test vectors (high‑dimensional)
    const int DIM = 1000;          // large, mostly empty dimension
    Vector vA(DIM);
    // Cluster a few non‑zero components near the start
    vA.put(0, 1.0f);
    vA.put(5, 0.3f);
    vA.put(12, -0.2f);

    Vector vB(DIM);
    // Place a single non‑zero component far from vA's cluster
    vB.put(950, 2.5f);

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
