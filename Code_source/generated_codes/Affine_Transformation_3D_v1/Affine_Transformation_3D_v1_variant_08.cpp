/* LLM input variant 8: sparse-skewed */
#include <cstdio>
#include <cmath>

// ------------------------------------------------------------
// Helper: fill a 4x4 matrix with scaling, rotation (around Z) and translation
// ------------------------------------------------------------
void compose_affine(float M[4][4], float sx, float sy, float sz,
                    float ang_deg, float tx, float ty, float tz)
{
    // Zero the matrix first
    int i, j;
    for (i = 0; i < 4; ++i)
        for (j = 0; j < 4; ++j)
            M[i][j] = 0.0f;

    // Convert angle to radians (expanded step)
    float rad = ang_deg * 3.14159265f / 180.0f;
    float c = cosf(rad);
    float s = sinf(rad);

    // Row‑major construction, scaling first, then rotation, finally translation
    // Scale‑X row
    M[0][0] = sx * c;   // scale X then rotate
    M[0][1] = -sx * s;  // rotate Z
    M[0][2] = 0.0f;
    M[0][3] = tx;       // translation X

    // Scale‑Y row
    M[1][0] = sy * s;   // rotate Z then scale Y
    M[1][1] = sy * c;
    M[1][2] = 0.0f;
    M[1][3] = ty;       // translation Y

    // Scale‑Z row (no rotation around Z)
    M[2][0] = 0.0f;
    M[2][1] = 0.0f;
    M[2][2] = sz;
    M[2][3] = tz;       // translation Z

    // Homogeneous row
    M[3][0] = 0.0f;
    M[3][1] = 0.0f;
    M[3][2] = 0.0f;
    M[3][3] = 1.0f;
}

// ------------------------------------------------------------
// Helper: apply a 4x4 matrix to a 3‑D point (homogeneous w=1)
// ------------------------------------------------------------
void transform_point(const float M[4][4],
                     const float src[4],
                     float dst[4])
{
    int r, c;
    // Explicitly compute each component with an inner loop
    for (r = 0; r < 4; ++r)
    {
        float acc = 0.0f;
        for (c = 0; c < 4; ++c)
            acc += M[r][c] * src[c];
        dst[r] = acc;               // store result
    }
}

// ------------------------------------------------------------
// Helper: print a 3‑D point (ignore homogeneous coordinate)
// ------------------------------------------------------------
void show_point(const char* label, const float p[4])
{
    std::printf("%s (%.2f, %.2f, %.2f)\n",
                label, p[0], p[1], p[2]);
}

// ------------------------------------------------------------
// Main driver – generates sparse‑skewed data and runs the transform
// ------------------------------------------------------------
int main()
{
    // --- 1. Build a sparse, skewed affine matrix -----------------
    // Edge cases: zero scaling on X and Z, normal scaling on Y,
    // no rotation, translation only on X (large positive)
    float mat[4][4];
    compose_affine(mat,
                   0.0f,          // scale X = 0 (collapse X dimension)
                   1.0f,          // scale Y = 1 (unchanged)
                   0.0f,          // scale Z = 0 (collapse Z dimension)
                   0.0f,          // rotate 0° around Z
                   9.9e5f, 0.0f,  // huge translation X, Y = 0
                   0.0f);         // translation Z = 0

    // --- 2. Prepare a stack array of sparsely populated test points ----------------
    const int PT_COUNT = 6;
    float src_pts[PT_COUNT][4];
    int idx;

    // Point 0: origin (all zeros)
    src_pts[0][0] = 0.0f; src_pts[0][1] = 0.0f; src_pts[0][2] = 0.0f; src_pts[0][3] = 1.0f;
    // Point 1: near‑zero point (tiny values)
    src_pts[1][0] = 1e-6f; src_pts[1][1] = -2e-6f; src_pts[1][2] = 3e-6f; src_pts[1][3] = 1.0f;
    // Point 2: pure Y axis (non‑zero only on Y)
    src_pts[2][0] = 0.0f; src_pts[2][1] = 42.0f; src_pts[2][2] = 0.0f; src_pts[2][3] = 1.0f;
    // Point 3: empty (all zeros except homogeneous)
    src_pts[3][0] = 0.0f; src_pts[3][1] = 0.0f; src_pts[3][2] = 0.0f; src_pts[3][3] = 1.0f;
    // Point 4: large X only (skewed far along X)
    src_pts[4][0] = 8.7e5f; src_pts[4][1] = 0.0f; src_pts[4][2] = 0.0f; src_pts[4][3] = 1.0f;
    // Point 5: mixed small values, mostly zero
    src_pts[5][0] = 0.0f; src_pts[5][1] = -0.001f; src_pts[5][2] = 0.0f; src_pts[5][3] = 1.0f;

    // --- 3. Transform each point using a loop‑heavy approach ----
    float tmp_res[4];
    for (idx = 0; idx < PT_COUNT; ++idx)
    {
        // Step A: copy source point to a local buffer (expanded step)
        float cur_src[4];
        int k;
        for (k = 0; k < 4; ++k)
            cur_src[k] = src_pts[idx][k];

        // Step B: apply affine transformation
        transform_point(mat, cur_src, tmp_res);

        // Step C: normalize homogeneous coordinate if needed (here it's always 1)
        if (tmp_res[3] != 0.0f && tmp_res[3] != 1.0f)
        {
            for (k = 0; k < 3; ++k)
                tmp_res[k] /= tmp_res[3];
            tmp_res[3] = 1.0f;
        }

        // Step D: output the result
        char label[32];
        std::sprintf(label, "P% d ->", idx);
        show_point(label, tmp_res);
    }

    return 0;
}
