/* LLM input variant 4: signed-extremes */
// Affine Transformation (3D) – version #10
// All code is placed inside main(), uses heap allocation, and only float/int types.

#include <iostream>
#include <iomanip>

int main()
{
    // ---------- 1. Define deterministic test vectors ----------
    // Four points to transform, mixing negative, zero, and positive values
    int pointCount = 4;
    float *srcPts = new float[pointCount * 4]; // x, y, z, w (w = 1)

    // Point 0 : (-1, 0, 0)
    srcPts[0] = -1.0f; srcPts[1] = 0.0f; srcPts[2] = 0.0f; srcPts[3] = 1.0f;
    // Point 1 : (0, 0, 0)
    srcPts[4] = 0.0f; srcPts[5] = 0.0f; srcPts[6] = 0.0f; srcPts[7] = 1.0f;
    // Point 2 : (0, -2, 0)
    srcPts[8] = 0.0f; srcPts[9] = -2.0f; srcPts[10] = 0.0f; srcPts[11] = 1.0f;
    // Point 3 : (1, 2, -3)
    srcPts[12] = 1.0f; srcPts[13] = 2.0f; srcPts[14] = -3.0f; srcPts[15] = 1.0f;

    // ---------- 2. Build a 4×4 affine matrix on the heap ----------
    // Scaling factors (mix of negative, zero, positive)
    float sX = -1.5f, sY = 0.0f, sZ = 2.5f;
    // Rotation about Z axis (45 degrees)
    float rad = 0.785398163f;               // π/4
    float sinA = 0.70710678f;               // sin(45°)
    float cosA = 0.70710678f;               // cos(45°)
    // Translation vector (negative, zero, positive)
    float tX = -3.0f, tY = 0.0f, tZ = 4.0f;

    // Allocate matrix (row‑major order)
    float *affine = new float[16];

    // Row 0
    affine[0]  = sX * cosA;   // m00
    affine[1]  = -sY * sinA;  // m01 (zero scaling yields zero)
    affine[2]  = 0.0f;        // m02
    affine[3]  = 0.0f;        // m03

    // Row 1
    affine[4]  = sX * sinA;   // m10
    affine[5]  = sY * cosA;   // m11 (zero)
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
