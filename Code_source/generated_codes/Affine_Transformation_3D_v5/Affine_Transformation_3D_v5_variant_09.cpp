#include <iostream>
#include <cmath>

/* LLM input variant 9: medium-deterministic-random */

// multiply two 4x4 matrices (stack arrays)
void mulM4(float a[4][4], float b[4][4], float out[4][4])
{
    int i = 0;
    while (i < 4)
    {
        int j = 0;
        while (j < 4)
        {
            out[i][j] = 0.0f;
            int k = 0;
            while (k < 4)
            {
                out[i][j] += a[i][k] * b[k][j];
                ++k;
            }
            ++j;
        }
        ++i;
    }
}

// multiply 4x4 matrix with 4‑vector (homogeneous coordinate)
void mulM4V4(float m[4][4], const float v[4], float r[4])
{
    int i = 0;
    while (i < 4)
    {
        // fused expression: sum of products in one statement
        r[i] = m[i][0] * v[0] + m[i][1] * v[1] + m[i][2] * v[2] + m[i][3] * v[3];
        ++i;
    }
}

// build scaling matrix
void scaleM(float s, float out[4][4])
{
    int i = 0;
    while (i < 4)
    {
        int j = 0;
        while (j < 4)
        {
            out[i][j] = (i == j) ? ((i < 3) ? s : 1.0f) : 0.0f;
            ++j;
        }
        ++i;
    }
}

// build rotation about Z axis
void rotZM(float a, float out[4][4])
{
    float ca = cosf(a);
    float sa = sinf(a);
    int i = 0;
    while (i < 4)
    {
        int j = 0;
        while (j < 4)
        {
            out[i][j] = 0.0f;
            ++j;
        }
        ++i;
    }
    out[0][0] =  ca; out[0][1] = -sa;
    out[1][0] =  sa; out[1][1] =  ca;
    out[2][2] = 1.0f;
    out[3][3] = 1.0f;
}

// build translation matrix
void translateM(float tx, float ty, float tz, float out[4][4])
{
    int i = 0;
    while (i < 4)
    {
        int j = 0;
        while (j < 4)
        {
            out[i][j] = (i == j) ? 1.0f : 0.0f;
            ++j;
        }
        ++i;
    }
    out[0][3] = tx;
    out[1][3] = ty;
    out[2][3] = tz;
}

// apply affine transformation to a list of 3‑D points
void applyAffine(const float pts[3][3], const float mat[4][4], float dst[3][3])
{
    int p = 0;
    while (p < 3)
    {
        float hom[4] = { pts[p][0], pts[p][1], pts[p][2], 1.0f };
        float res[4];
        mulM4V4(const_cast<float(*)[4]>(mat), hom, res);
        // fuse division by w (here w == 1, but kept for completeness)
        dst[p][0] = res[0] / res[3];
        dst[p][1] = res[1] / res[3];
        dst[p][2] = res[2] / res[3];
        ++p;
    }
}

int main()
{
    // predefined points (deterministic pseudo‑random pattern)
    float pts[3][3] = {
        { 1.5f, -2.0f,  0.5f},
        {-0.5f,  3.3f, -1.1f},
        { 2.2f,  0.0f, -3.3f}
    };

    // parameters for affine transform
    float scaleFactor = 1.7320508f;                 // sqrt(3) ~ 1.732
    float angleRad    = 3.1415926f / 3.0f;          // 60 degrees
    float tx = -4.5f, ty = 7.2f, tz = -1.3f;

    // build component matrices
    float S[4][4], R[4][4], T[4][4];
    scaleM(scaleFactor, S);
    rotZM(angleRad, R);
    translateM(tx, ty, tz, T);

    // combine: M = T * R * S  (order matters)
    float RS[4][4];
    mulM4(R, S, RS);
    float M[4][4];
    mulM4(T, RS, M);

    // transform points
    float outPts[3][3];
    applyAffine(pts, M, outPts);

    // print results
    int i = 0;
    while (i < 3)
    {
        std::cout << "Point " << i << " -> ("
                  << outPts[i][0] << ", "
                  << outPts[i][1] << ", "
                  << outPts[i][2] << ")\n";
        ++i;
    }
    return 0;
}
