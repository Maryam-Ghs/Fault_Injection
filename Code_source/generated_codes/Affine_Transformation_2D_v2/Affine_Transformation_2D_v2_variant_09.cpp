#include <iostream>
#include <cmath>

/* LLM input variant 9: medium-deterministic-random */

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
    // Deterministic pseudo‑random point set (medium size)
    const int ptCount = 7;
    Pt2D* original = allocatePts(ptCount);
    Pt2D* transformed = allocatePts(ptCount);

    // Fill points with a simple linear‑congruential pattern
    for (int i = 0; i < ptCount; ++i)
    {
        // Values spread across negative, zero, small, and large magnitudes
        original[i].x = static_cast<float>((i * 31) % 23) - 11.5f;   // range approx -11.5 .. +11.5
        original[i].y = static_cast<float>((i * 47) % 29) - 14.0f;   // range approx -14 .. +14
        // Inject two extreme points
        if (i == 3) { original[i].x = 5e5f; original[i].y = -5e5f; }
        if (i == 5) { original[i].x = -2e6f; original[i].y = 2e6f; }
    }

    // Parameter sets (scale, shear, rotate, translate)
    struct Param { float sx, sy, shx, shy, ang, tx, ty; };
    const int caseCnt = 5;
    Param cases[caseCnt] = {
        // 1) Small uniform scaling, slight shear, 30° rotation, modest translation
        {0.75f, 0.75f, 0.1f, -0.1f, 30.0f,  3.0f, -2.0f},
        // 2) Mixed scaling (one axis inverted), 120° rotation, no translation
        {-1.2f, 0.8f, 0.0f, 0.0f, 120.0f, 0.0f, 0.0f},
        // 3) Large shear, 270° rotation, large translation
        {1.0f, 1.0f, 4.0f, 3.0f, 270.0f, -8e4f, 9e4f},
        // 4) Zero scaling on X, normal Y scaling, 45° rotation, small translation
        {0.0f, 1.5f, 0.0f, 0.0f, 45.0f,  1.0f,  1.0f},
        // 5) Uniform scaling >1, 60° rotation, negative translation
        {2.5f, 2.5f, 0.0f, 0.0f, 60.0f, -15.0f, -25.0f}
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
