/* LLM input variant 3: zeros-and-ones */
// Affine Transformation (3D) – version #8
// All code resides inside main, uses heap allocation, manual unrolling, and deterministic data.

#include <iostream>

int main()
{
    // ---------- allocate and fill source points (3 points, 3 coordinates each) ----------
    int pointCount = 3;
    float* src = new float[pointCount * 3];   // [x0,y0,z0, x1,y1,z1, x2,y2,z2]

    // deterministic test vectors emphasizing zeros and ones
    src[0] = 0.0f; src[1] = 0.0f; src[2] = 0.0f;   // point 0 (origin)
    src[3] = 1.0f; src[4] = 1.0f; src[5] = 1.0f;   // point 1 (unit cube corner)
    src[6] = 0.0f; src[7] = 1.0f; src[8] = 0.0f;   // point 2 (mixed)

    // ---------- allocate and fill 4x4 affine matrix (row‑major) ----------
    float* trMat = new float[4 * 4];
    // identity matrix (ones on diagonal, zeros elsewhere)
    trMat[0] = 1.0f;  trMat[1] = 0.0f;  trMat[2] = 0.0f;  trMat[3] = 0.0f;
    trMat[4] = 0.0f;  trMat[5] = 1.0f;  trMat[6] = 0.0f;  trMat[7] = 0.0f;
    trMat[8] = 0.0f;  trMat[9] = 0.0f;  trMat[10] = 1.0f; trMat[11] = 0.0f;
    trMat[12] = 0.0f; trMat[13] = 0.0f; trMat[14] = 0.0f; trMat[15] = 1.0f;

    // ---------- allocate result array ----------
    float* dst = new float[pointCount * 3]; // transformed points

    // ---------- manual loop unrolling for each point ----------
    // Point 0
    float x0 = src[0];
    float y0 = src[1];
    float z0 = src[2];
    float tx0 = trMat[0] * x0 + trMat[1] * y0 + trMat[2] * z0 + trMat[3];
    float ty0 = trMat[4] * x0 + trMat[5] * y0 + trMat[6] * z0 + trMat[7];
    float tz0 = trMat[8] * x0 + trMat[9] * y0 + trMat[10] * z0 + trMat[11];
    dst[0] = tx0;
    dst[1] = ty0;
    dst[2] = tz0;

    // Point 1
    float x1 = src[3];
    float y1 = src[4];
    float z1 = src[5];
    float tx1 = trMat[0] * x1 + trMat[1] * y1 + trMat[2] * z1 + trMat[3];
    float ty1 = trMat[4] * x1 + trMat[5] * y1 + trMat[6] * z1 + trMat[7];
    float tz1 = trMat[8] * x1 + trMat[9] * y1 + trMat[10] * z1 + trMat[11];
    dst[3] = tx1;
    dst[4] = ty1;
    dst[5] = tz1;

    // Point 2
    float x2 = src[6];
    float y2 = src[7];
    float z2 = src[8];
    float tx2 = trMat[0] * x2 + trMat[1] * y2 + trMat[2] * z2 + trMat[3];
    float ty2 = trMat[4] * x2 + trMat[5] * y2 + trMat[6] * z2 + trMat[7];
    float tz2 = trMat[8] * x2 + trMat[9] * y2 + trMat[10] * z2 + trMat[11];
    dst[6] = tx2;
    dst[7] = ty2;
    dst[8] = tz2;

    // ---------- output ----------
    std::cout << "Original -> Transformed\n";
    std::cout << "---------------------------\n";
    std::cout << src[0] << " " << src[1] << " " << src[2] << " -> "
              << dst[0] << " " << dst[1] << " " << dst[2] << "\n";
    std::cout << src[3] << " " << src[4] << " " << src[5] << " -> "
              << dst[3] << " " << dst[4] << " " << dst[5] << "\n";
    std::cout << src[6] << " " << src[7] << " " << src[8] << " -> "
              << dst[6] << " " << dst[7] << " " << dst[8] << "\n";

    // ---------- cleanup ----------
    delete[] src;
    delete[] trMat;
    delete[] dst;

    return 0;
}
