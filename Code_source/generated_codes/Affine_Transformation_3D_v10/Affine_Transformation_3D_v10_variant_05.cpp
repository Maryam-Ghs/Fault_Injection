// LLM input variant 5: duplicate-heavy
// Affine Transformation (3D) – version #10
// All code is placed inside main(), uses heap allocation, and only float/int types.

#include <iostream>
#include <iomanip>

int main()
{
    // ---------- 1. Define deterministic test vectors ----------
    // Six points, three distinct patterns each repeated twice
    int pointCount = 6;
    float *srcPts = new float[pointCount * 4]; // x, y, z, w (w = 1)

    // Pattern A : (1, 2, 3)
    srcPts[0]  = 1.0f; srcPts[1]  = 2.0f; srcPts[2]  = 3.0f; srcPts[3]  = 1.0f;
    srcPts[4]  = 1.0f; srcPts[5]  = 2.0f; srcPts[6]  = 3.0f; srcPts[7]  = 1.0f;

    // Pattern B : (4, 5, 6)
    srcPts[8]  = 4.0f; srcPts[9]  = 5.0f; srcPts[10] = 6.0f; srcPts[11] = 1.0f;
    srcPts[12] = 4.0f; srcPts[13] = 5.0f; srcPts[14] = 6.0f; srcPts[15] = 1.0f;

    // Pattern C : (7, 8, 9)
    srcPts[16] = 7.0f; srcPts[17] = 8.0f; srcPts[18] = 9.0f; srcPts[19] = 1.0f;
    srcPts[20] = 7.0f; srcPts[21] = 8.0f; srcPts[22] = 9.0f; srcPts[23] = 1.0f;

    // ---------- 2. Build a 4×4 affine matrix on the heap ----------
    // Uniform scaling
    float sX = 2.0f, sY = 2.0f, sZ = 2.0f;
    // No rotation (identity)
    float rad = 0.0f;
    float sinA = 0.0f;
    float cosA = 1.0f;
    // Uniform translation
    float tX = 3.0f, tY = 3.0f, tZ = 3.0f;

    // Allocate matrix (row‑major order)
    float *affine = new float[16];

    // Row 0
    affine[0]  = sX * cosA;   // m00 = 2.0
    affine[1]  = -sY * sinA;  // m01 = -0.0
    affine[2]  = 0.0f;        // m02
    affine[3]  = 0.0f;        // m03

    // Row 1
    affine[4]  = sX * sinA;   // m10 = 0.0
    affine[5]  = sY * cosA;   // m11 = 2.0
    affine[6]  = 0.0f;        // m12
    affine[7]  = 0.0f;        // m13

    // Row 2
    affine[8]  = 0.0f;        // m20
    affine[9]  = 0.0f;        // m21
    affine[10] = sZ;          // m22 = 2.0
    affine[11] = 0.0f;        // m23

    // Row 3 (translation)
    affine[12] = tX;          // m30 = 3.0
    affine[13] = tY;          // m31 = 3.0
    affine[14] = tZ;          // m32 = 3.0
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
    std::cout << "Affine Transformation (3D) – deterministic test vectors\n";
    std::cout << "-------------------------------------------------------\n";

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
