/* LLM input variant 10: large-safe-stress */
// Affine Transformation (3D) – version #8
// All code resides inside main, uses heap allocation, deterministic data, and scalable loops.

#include <iostream>

int main()
{
    // ---------- allocate and fill source points (large number, 3 coordinates each) ----------
    const int pointCount = 1000;               // increased from 3 to 1000
    const int coordPerPoint = 3;
    float* src = new float[pointCount * coordPerPoint];   // [x0,y0,z0, x1,y1,z1, ...]

    // deterministic test vectors: simple arithmetic progression
    for (int i = 0; i < pointCount; ++i) {
        src[i * 3 + 0] = static_cast<float>(i) * 1.0f;      // x
        src[i * 3 + 1] = static_cast<float>(i) * -2.0f;     // y
        src[i * 3 + 2] = static_cast<float>(i) * 3.0f;      // z
    }

    // ---------- allocate and fill 4x4 affine matrix (row‑major) ----------
    float* trMat = new float[4 * 4];
    // scaling (2,3,4) + translation (5,6,7)
    trMat[0] = 2.0f;  trMat[1] = 0.0f;  trMat[2] = 0.0f;  trMat[3] = 5.0f;
    trMat[4] = 0.0f;  trMat[5] = 3.0f;  trMat[6] = 0.0f;  trMat[7] = 6.0f;
    trMat[8] = 0.0f;  trMat[9] = 0.0f;  trMat[10] = 4.0f; trMat[11] = 7.0f;
    trMat[12] = 0.0f; trMat[13] = 0.0f; trMat[14] = 0.0f; trMat[15] = 1.0f;

    // ---------- allocate result array ----------
    float* dst = new float[pointCount * coordPerPoint]; // transformed points

    // ---------- loop over all points ----------
    for (int i = 0; i < pointCount; ++i) {
        float x = src[i * 3 + 0];
        float y = src[i * 3 + 1];
        float z = src[i * 3 + 2];

        float tx = trMat[0] * x + trMat[1] * y + trMat[2] * z + trMat[3];
        float ty = trMat[4] * x + trMat[5] * y + trMat[6] * z + trMat[7];
        float tz = trMat[8] * x + trMat[9] * y + trMat[10] * z + trMat[11];

        dst[i * 3 + 0] = tx;
        dst[i * 3 + 1] = ty;
        dst[i * 3 + 2] = tz;
    }

    // ---------- output ----------
    std::cout << "Original -> Transformed\n";
    std::cout << "---------------------------\n";
    for (int i = 0; i < pointCount; ++i) {
        std::cout << src[i * 3 + 0] << " " << src[i * 3 + 1] << " " << src[i * 3 + 2] << " -> "
                  << dst[i * 3 + 0] << " " << dst[i * 3 + 1] << " " << dst[i * 3 + 2] << "\n";
    }

    // ---------- cleanup ----------
    delete[] src;
    delete[] trMat;
    delete[] dst;

    return 0;
}
