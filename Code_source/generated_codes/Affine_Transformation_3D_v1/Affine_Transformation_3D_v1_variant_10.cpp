#include <cstdio>
#include <cmath>

/* LLM input variant 10: large-safe-stress */

// ------------------------------------------------------------
// Helper: fill a 4x4 matrix with scaling, rotation (around Z) and translation
// ------------------------------------------------------------
void compose_affine(float M[4][4], float sx, float sy, float sz,
                    float ang_deg, float tx, float ty, float tz)
{
    // Zero the matrix first
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            M[i][j] = 0.0f;

    // Convert angle to radians
    float rad = ang_deg * 3.14159265f / 180.0f;
    float c = cosf(rad);
    float s = sinf(rad);

    // Row‑major construction, scaling first, then rotation, finally translation
    M[0][0] = sx * c;   // scale X then rotate
    M[0][1] = -sx * s;  // rotate Z
    M[0][2] = 0.0f;
    M[0][3] = tx;       // translation X

    M[1][0] = sy * s;   // rotate Z then scale Y
    M[1][1] = sy * c;
    M[1][2] = 0.0f;
    M[1][3] = ty;       // translation Y

    M[2][0] = 0.0f;
    M[2][1] = 0.0f;
    M[2][2] = sz;
    M[2][3] = tz;       // translation Z

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
    for (int r = 0; r < 4; ++r)
    {
        float acc = 0.0f;
        for (int c = 0; c < 4; ++c)
            acc += M[r][c] * src[c];
        dst[r] = acc;
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
// Main driver – generates a larger safe stress dataset and runs the transform
// ------------------------------------------------------------
int main()
{
    // --- 1. Build a pathological affine matrix -----------------
    // Edge cases: zero scaling on Y, huge translation, 270° rotation
    float mat[4][4];
    compose_affine(mat,
                   1.0f,          // scale X = 1 (normal)
                   0.0f,          // scale Y = 0 (collapse dimension)
                   10.0f,         // scale Z = 10 (moderately large)
                   270.0f,        // rotate 270° around Z
                   1e8f, -1e8f,   // huge translation X,Y
                   -1e8f);        // huge translation Z

    // --- 2. Prepare a larger stack array of test points ---------
    const int PT_COUNT = 1000;
    float src_pts[PT_COUNT][4];

    // Populate points deterministically
    for (int i = 0; i < PT_COUNT; ++i)
    {
        // Ensure homogeneous coordinate is 1
        src_pts[i][3] = 1.0f;

        // Use a simple pattern that covers positive, negative and zero values
        // and grows linearly with the index to stress the transformation.
        src_pts[i][0] = (float)(i % 200 - 100);           // range [-100, 99]
        src_pts[i][1] = (float)((i * 3) % 400 - 200);    // range [-200, 199]
        src_pts[i][2] = (float)((i * 5) % 600 - 300);    // range [-300, 299]

        // Insert a few explicit edge cases at known positions
        if (i == 0) { src_pts[i][0] = 0.0f; src_pts[i][1] = 0.0f; src_pts[i][2] = 0.0f; }
        if (i == 1) { src_pts[i][0] = 1.0f; src_pts[i][1] = 0.0f; src_pts[i][2] = 0.0f; }
        if (i == 2) { src_pts[i][0] = 0.0f; src_pts[i][1] = 1.0f; src_pts[i][2] = 0.0f; }
        if (i == 3) { src_pts[i][0] = 0.0f; src_pts[i][1] = 0.0f; src_pts[i][2] = 1.0f; }
        if (i == 4) { src_pts[i][0] = 1e5f; src_pts[i][1] = -2e5f; src_pts[i][2] = 3e5f; }
        if (i == 5) { src_pts[i][0] = -0.75f; src_pts[i][1] = 0.5f; src_pts[i][2] = -0.25f; }
    }

    // --- 3. Transform each point using a loop‑heavy approach ----
    float tmp_res[4];
    for (int idx = 0; idx < PT_COUNT; ++idx)
    {
        // Step A: copy source point to a local buffer
        float cur_src[4];
        for (int k = 0; k < 4; ++k)
            cur_src[k] = src_pts[idx][k];

        // Step B: apply affine transformation
        transform_point(mat, cur_src, tmp_res);

        // Step C: normalize homogeneous coordinate if needed
        if (tmp_res[3] != 0.0f && tmp_res[3] != 1.0f)
        {
            for (int k = 0; k < 3; ++k)
                tmp_res[k] /= tmp_res[3];
            tmp_res[3] = 1.0f;
        }

        // Step D: output the result (label padded for readability)
        char label[32];
        std::sprintf(label, "P%4d ->", idx);
        show_point(label, tmp_res);
    }

    return 0;
}
