// LLM input variant 7: reverse-adversarial
// Affine Transformation (3D) – version #10
// All code is placed inside main(), uses heap allocation, and only float/int types.

#include <iostream>
#include <iomanip>

int main()
{
    // ---------- 1. Define deterministic test vectors ----------
    // Eight points (cube corners) in reverse lexicographic order
    int pointCount = 8;
    float *srcPts = new float[pointCount * 4]; // x, y, z, w (w = 1)

    // Point 7 : (1, 1, 1)
    srcPts[0]  = 1.0f; srcPts[1]  = 1.0f; srcPts[2]  = 1.0f; srcPts[3]  = 1.0f;
    // Point 6 : (1, 1, 0)
    srcPts[4]  = 1.0f; srcPts[5]  = 1.0f; srcPts[6]  = 0.0f; srcPts[7]  = 1.0f;
    // Point 5 : (1, 0, 1)
    srcPts[8]  = 1.0f; srcPts[9]  = 0.0f; srcPts[10] = 1.0f; srcPts[11] = 1.0f;
    // Point 4 : (1, 0, 0)
    srcPts[12] = 1.0f; srcPts[13] = 0.0f; srcPts[14] = 0.0f; srcPts[15] = 1.0f;
    // Point 3 : (0, 1, 1)
    srcPts[16] = 0.0f; srcPts[17] = 1.0f; srcPts[18] = 1.0f; srcPts[19] = 1.0f;
    // Point 2 : (0, 1, 0)
    srcPts[20] = 0.0f; srcPts[21] = 1.0f; srcPts[22] = 0.0f; srcPts[23] = 1.0f;
    // Point 1 : (0, 0, 1)
    srcPts[24] = 0.0f; srcPts[25] = 0.0f; srcPts[26] = 1.0f; srcPts[27] = 1.0f;
    // Point 0 : (0, 0, 0)
    srcPts[28] = 0.0f; srcPts[29] = 0.0f; srcPts[30] = 0.0f; srcPts[31] = 1.0f;

    // ---------- 2. Build a 4×4 affine matrix on the heap ----------
    // Scaling factors (large and sign‑flipping)
    float sX = 1000.0f, sY = -1000.0f, sZ = 1000.0f;
    // Rotation about Z axis (180 degrees)
    float rad = 3.14159265f;               // π
    float sinA = 0.0f;                     // sin(π) ≈ 0
    float cosA = -1.0f;                    // cos(π) = -1
    // Translation vector (large offsets)
    float tX = -1000.0f, tY = 1000.0f, tZ = -1000.0f;

    // Allocate matrix (row‑major order)
    float *affine = new float[16];

    // Row 0
    affine[0]  = sX * cosA;   // m00
    affine[1]  = -sY * sinA;  // m01
    affine[2]  = 0.0f;        // m02
    affine[3]  = 0.0f;        // m03

    // Row 1
    affine[4]  = sX * sinA;   // m10
    affine[5]  = sY * cosA;   // m11
    affine[6]  = 0.0f;        // m12
    affine[7]  = 0.0f;        // m13

    // Row 2
    affine[8]  = 0.0f;        // m20
    affine[9]  = 0.0f;        // m21
    affine[10] = sZ;          // m22
    affine[11] = 0.0f;        // m23

    // Row 3 (translation)
    affine[12] = tX;          // m30
    affine[13] = tY;          // m31
    affine[14] = tZ;          // m32
    affine[15] = 1.0f;        // m33

    // ---------- 3. Allocate space for transformed points ----------
    float *dstPts = new float[pointCount * 4];

    // ---------- 4. Perform matrix‑vector multiplication ----------
    int i = 0;
    while (i < pointCount)
    {
        // Load source point components
        float vx = srcPts[i * 4 + 0];
        float vy = srcPts[i * 4 + 1];
        float vz = srcPts[i * 4 + 2];
        float vw = srcPts[i * 4 + 3];   // always 1

        // Compute each component of the result with explicit temporaries
        float tmp0 = affine[0] * vx + affine[1] * vy + affine[2] * vz + affine[3] * vw;
        float tmp1 = affine[4] * vx + affine[5] * vy + affine[6] * vz + affine[7] * vw;
        float tmp2 = affine[8] * vx + affine[9] * vy + affine[10] * vz + affine[11] * vw;
        float tmp3 = affine[12] * vx + affine[13] * vy + affine[14] * vz + affine[15] * vw;

        // Store result
        dstPts[i * 4 + 0] = tmp0;
        dstPts[i * 4 + 1] = tmp1;
        dstPts[i * 4 + 2] = tmp2;
        dstPts[i * 4 + 3] = tmp3;   // should remain 1

        ++i;
    }

    // ---------- 5. Print original and transformed points ----------
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "Affine Transformation (3D) – reverse‑adversarial test vectors\n";
    std::cout << "---------------------------------------------------------------\n";

    int idx = 0;
    while (idx < pointCount)
    {
        std::cout << "Point " << idx << " : ("
                  << srcPts[idx * 4 + 0] << ", "
                  << srcPts[idx * 4 + 1] << ", "
                  << srcPts[idx * 4 + 2] << ")  =>  ("
                  << dstPts[idx * 4 + 0] << ", "
                  << dstPts[idx * 4 + 1] << ", "
                  << dstPts[idx * 4 + 2] << ")\n";
        ++idx;
    }

    // ---------- 6. Clean up ----------
    delete[] srcPts;
    delete[] dstPts;
    delete[] affine;

    return 0;
}
