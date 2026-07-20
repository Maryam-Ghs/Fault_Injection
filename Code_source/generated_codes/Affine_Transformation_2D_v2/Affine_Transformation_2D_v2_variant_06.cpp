/* LLM input variant 6: ordered-structured */
#include <iostream>
#include <cmath>

// ---------- Helper structures ----------
struct Pt2D
{
    float x;
    float y;
};

// ---------- Memory allocation ----------
Pt2D* allocatePts(int cnt)
{
    Pt2D* arr = new Pt2D[cnt];
    return arr;
}

float* allocateAff()
{
    // 2x3 matrix stored row‑major: [a00 a01 tx; a10 a11 ty]
    float* m = new float[6];
    return m;
}

// ---------- Build affine matrix (scale → shear → rotate → translate) ----------
void composeAffine(float* mat,
                   float sx, float sy,
                   float shx, float shy,
                   float angDeg,
                   float trX, float trY)
{
    // 1. start with identity
    mat[0] = 1.0f; mat[1] = 0.0f; mat[2] = 0.0f;
    mat[3] = 0.0f; mat[4] = 1.0f; mat[5] = 0.0f;

    // 2. scaling
    mat[0] *= sx; mat[1] *= sx;
    mat[3] *= sy; mat[4] *= sy;

    // 3. shear (horizontal then vertical)
    float tmp00 = mat[0] + shx * mat[3];
    float tmp01 = mat[1] + shx * mat[4];
    float tmp10 = shy * mat[0] + mat[3];
    float tmp11 = shy * mat[1] + mat[4];
    mat[0] = tmp00; mat[1] = tmp01;
    mat[3] = tmp10; mat[4] = tmp11;

    // 4. rotation (angle in degrees)
    float rad = angDeg * 3.1415926f / 180.0f;
    float c = static_cast<float>(cos(rad));
    float s = static_cast<float>(sin(rad));

    tmp00 =  c * mat[0] + s * mat[3];
    tmp01 =  c * mat[1] + s * mat[4];
    tmp10 = -s * mat[0] + c * mat[3];
    tmp11 = -s * mat[1] + c * mat[4];
    mat[0] = tmp00; mat[1] = tmp01;
    mat[3] = tmp10; mat[4] = tmp11;

    // 5. translation (added after linear part)
    mat[2] = trX;
    mat[5] = trY;
}

// ---------- Apply affine transformation ----------
void applyAffine(const Pt2D* src, Pt2D* dst, int cnt, const float* mat)
{
    int i = 0;
    while (i < cnt)
    {
        // x' = a00*x + a01*y + tx
        // y' = a10*x + a11*y + ty
        float xv = src[i].x;
        float yv = src[i].y;

        float tx = mat[0] * xv + mat[1] * yv + mat[2];
        float ty = mat[3] * xv + mat[4] * yv + mat[5];

        dst[i].x = tx;
        dst[i].y = ty;
        ++i;
    }
}

// ---------- Print points ----------
void dumpPts(const Pt2D* pts, int cnt)
{
    int i = 0;
    while (i < cnt)
    {
        std::cout << "(" << pts[i].x << ", " << pts[i].y << ")";
        if (i + 1 < cnt) std::cout << " -> ";
        ++i;
    }
    std::cout << std::endl;
}

// ---------- Main driver ----------
int main()
{
    // Ordered, symmetric test set
    const int ptCount = 8;
    Pt2D* original = allocatePts(ptCount);
    Pt2D* transformed = allocatePts(ptCount);

    // Populate original points in a monotonic increasing sequence
    for (int i = 0; i < ptCount; ++i)
    {
        original[i].x = static_cast<float>(i - 3);   // -3, -2, -1, 0, 1, 2, 3, 4
        original[i].y = static_cast<float>(i - 3);
    }

    // Parameter sets (scale, shear, rotate, translate) – ordered progression
    struct Param { float sx, sy, shx, shy, ang, tx, ty; };
    const int caseCnt = 3;
    Param cases[caseCnt] = {
        // 1) Uniform scaling by 1 (identity)
        {1.0f, 1.0f, 0.0f, 0.0f,   0.0f, 0.0f, 0.0f},
        // 2) Moderate scaling with small shear, 30° rotation, modest translation
        {1.5f, 0.8f, 0.1f, -0.1f, 30.0f, 2.0f, -2.0f},
        // 3) Larger scaling, no shear, 90° rotation, symmetric translation
        {2.0f, 2.0f, 0.0f, 0.0f, 90.0f, -5.0f, 5.0f}
    };

    for (int idx = 0; idx < caseCnt; ++idx)
    {
        float* mat = allocateAff();
        composeAffine(mat,
                      cases[idx].sx, cases[idx].sy,
                      cases[idx].shx, cases[idx].shy,
                      cases[idx].ang,
                      cases[idx].tx, cases[idx].ty);

        applyAffine(original, transformed, ptCount, mat);

        std::cout << "Case " << idx + 1 << ":\n";
        std::cout << "  Input : ";  dumpPts(original, ptCount);
        std::cout << "  Output: ";  dumpPts(transformed, ptCount);
        std::cout << std::endl;

        delete[] mat;
    }

    delete[] original;
    delete[] transformed;
    return 0;
}
