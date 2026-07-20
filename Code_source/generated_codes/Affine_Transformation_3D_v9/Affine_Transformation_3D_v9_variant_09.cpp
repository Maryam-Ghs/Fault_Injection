/* LLM input variant 9: medium-deterministic-random */
// Affine Transformation (3D) – version #9
// Implements scaling, rotation (Z‑axis) and translation on a medium‑sized
// deterministic pseudo‑random point set.

#include <iostream>
#include <cmath>
#include <cstdint>

// ------------------------------------------------------------
// Allocate a 4×4 matrix (row‑major) and fill it with the desired
// affine transformation: scale‑1.5, rotate‑30° about Z, translate (‑2,5,‑1).
// ------------------------------------------------------------
float* buildMtx()
{
    // 4×4 matrix on the heap
    float* mtx = new float[16];

    // Identity base
    for (int i = 0; i < 16; ++i) mtx[i] = (i % 5 == 0) ? 1.0f : 0.0f;

    // ---------- scaling ----------
    float s = 1.5f;               // uniform scale factor
    for (int i = 0; i < 3; ++i)   // scale the upper‑left 3×3 block
        mtx[i * 4 + i] *= s;

    // ---------- rotation about Z ----------
    float ang = 30.0f * 3.14159265f / 180.0f; // convert to radians
    float c = cosf(ang);
    float s2 = sinf(ang);
    // replace the first two rows / columns (still 3×3)
    mtx[0] = c * s;  mtx[1] = -s2 * s;  // row 0
    mtx[4] = s2 * s;  mtx[5] =  c * s;   // row 1

    // ---------- translation ----------
    mtx[3]  = -2.0f;   // tx
    mtx[7]  =  5.0f;   // ty
    mtx[11] = -1.0f;   // tz

    return mtx;
}

// ------------------------------------------------------------
// Allocate an array of 3‑D points (x,y,z) on the heap and fill it
// with a deterministic pseudo‑random sequence.
// ------------------------------------------------------------
float* spawnPts(int cnt)
{
    float* pts = new float[cnt * 3];
    // Simple linear congruential generator (LCG) with fixed seed
    uint32_t seed = 123456789u;
    const uint32_t a = 1664525u;
    const uint32_t c = 1013904223u;

    for (int i = 0; i < cnt * 3; ++i)
    {
        seed = a * seed + c;
        // Map to range [-10.0, 10.0]
        float val = static_cast<float>(seed & 0xFFFFu) / 65535.0f; // [0,1]
        val = val * 20.0f - 10.0f;                                 // [-10,10]
        pts[i] = val;
    }
    return pts;
}

// ------------------------------------------------------------
// Apply the affine matrix to every point in the array.
// The computation is fused into three expressions per point.
// ------------------------------------------------------------
void warpPts(const float* mtx, float* pts, int cnt)
{
    int i = 0;
    while (i < cnt)
    {
        // fetch original coordinates
        float x = pts[i * 3];
        float y = pts[i * 3 + 1];
        float z = pts[i * 3 + 2];

        // fused affine transformation (row‑major matrix)
        float nx = mtx[0] * x + mtx[1] * y + mtx[2] * z + mtx[3];
        float ny = mtx[4] * x + mtx[5] * y + mtx[6] * z + mtx[7];
        float nz = mtx[8] * x + mtx[9] * y + mtx[10] * z + mtx[11];

        // store results back
        pts[i * 3]     = nx;
        pts[i * 3 + 1] = ny;
        pts[i * 3 + 2] = nz;

        ++i;
    }
}

// ------------------------------------------------------------
// Print the transformed points.
// ------------------------------------------------------------
void showPts(const float* pts, int cnt)
{
    int i = 0;
    while (i < cnt)
    {
        std::cout << "P" << i << ": ("
                  << pts[i * 3] << ", "
                  << pts[i * 3 + 1] << ", "
                  << pts[i * 3 + 2] << ")\n";
        ++i;
    }
}

// ------------------------------------------------------------
int main()
{
    // number of points to transform
    int num = 10;

    // build transformation matrix and point set
    float* mtx = buildMtx();
    float* pts = spawnPts(num);

    // apply affine transformation
    warpPts(mtx, pts, num);

    // output the results
    showPts(pts, num);

    // clean up heap memory
    delete[] mtx;
    delete[] pts;

    return 0;
}
