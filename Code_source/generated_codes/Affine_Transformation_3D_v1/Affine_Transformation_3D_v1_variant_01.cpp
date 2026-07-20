#include <cstdio>
#include <cmath>

/* LLM input variant 1: minimal-boundary */

// ------------------------------------------------------------
// Helper: fill a 4x4 matrix with scaling, rotation (around Z) and translation
// ------------------------------------------------------------
void compose_affine(float M[4][4], float sx, float sy, float sz,
                    float ang_deg, float tx, float ty, float tz)
{
    int i, j;
    for (i = 0; i < 4; ++i)
        for (j = 0; j < 4; ++j)
            M[i][j] = 0.0f;

    float rad = ang_deg * 3.14159265f / 180.0f;
    float c = cosf(rad);
    float s = sinf(rad);

    M[0][0] = sx * c;
    M[0][1] = -sx * s;
    M[0][2] = 0.0f;
    M[0][3] = tx;

    M[1][0] = sy * s;
    M[1][1] = sy * c;
    M[1][2] = 0.0f;
    M[1][3] = ty;

    M[2][0] = 0.0f;
    M[2][1] = 0.0f;
    M[2][2] = sz;
    M[2][3] = tz;

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
    for (r = 0; r < 4; ++r)
    {
        float acc = 0.0f;
        for (c = 0; c < 4; ++c)
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
// Main driver – minimal‑boundary data
// ------------------------------------------------------------
int main()
{
    float mat[4][4];
    compose_affine(mat,
                   1.0f,   // scale X
                   0.0f,   // scale Y (boundary: zero)
                   1.0f,   // scale Z
                   0.0f,   // rotation 0°
                   0.0f, 0.0f, 0.0f); // zero translation

    const int PT_COUNT = 1;
    float src_pts[PT_COUNT][4];

    // Single test point: origin
    src_pts[0][0] = 0.0f; src_pts[0][1] = 0.0f; src_pts[0][2] = 0.0f; src_pts[0][3] = 1.0f;

    float tmp_res[4];
    for (int idx = 0; idx < PT_COUNT; ++idx)
    {
        float cur_src[4];
        for (int k = 0; k < 4; ++k)
            cur_src[k] = src_pts[idx][k];

        transform_point(mat, cur_src, tmp_res);

        if (tmp_res[3] != 0.0f && tmp_res[3] != 1.0f)
        {
            for (int k = 0; k < 3; ++k)
                tmp_res[k] /= tmp_res[3];
            tmp_res[3] = 1.0f;
        }

        char label[32];
        std::sprintf(label, "P% d ->", idx);
        show_point(label, tmp_res);
    }

    return 0;
}
