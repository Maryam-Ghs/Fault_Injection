/* LLM input variant 9: medium-deterministic-random */
// Affine Transformation 2D – version #6
// No external input, deterministic test vectors, stack allocation only.

#include <iostream>
#include <iomanip>
#include <cmath>

// Simple 2‑D point
struct Pt2 {
    float x;
    float y;
};

// 2×3 affine matrix stored row‑major: [ a b tx ; c d ty ]
struct Aff2 {
    float m[2][3];
};

// -------------------------------------------------------------------
// Helper: create a rotation‑plus‑translation matrix
// angle in degrees, translation (dx,dy)
void buildTransform(Aff2& out, float angleDeg, float dx, float dy)
{
    // convert to radians (reordered arithmetic)
    float rad = angleDeg * 3.14159265f / 180.0f;

    // sine and cosine
    float s = std::sin(rad);
    float c = std::cos(rad);

    // fill matrix (order of assignments changed)
    out.m[0][0] =  c;   // a
    out.m[0][1] = -s;   // b
    out.m[0][2] =  dx;  // tx
    out.m[1][0] =  s;   // c
    out.m[1][1] =  c;   // d
    out.m[1][2] =  dy;  // ty
}

// -------------------------------------------------------------------
// Helper: apply one affine matrix to a point
Pt2 transformPoint(const Aff2& mat, const Pt2& p)
{
    Pt2 r;
    // reordered arithmetic: compute x part first, then y
    r.x = mat.m[0][0] * p.x + mat.m[0][1] * p.y + mat.m[0][2];
    r.y = mat.m[1][0] * p.x + mat.m[1][1] * p.y + mat.m[1][2];
    return r;
}

// -------------------------------------------------------------------
// Helper: combine two affine transforms (result = B ∘ A)
void combine(const Aff2& A, const Aff2& B, Aff2& out)
{
    // compute upper‑left 2×2 block
    out.m[0][0] = B.m[0][0] * A.m[0][0] + B.m[0][1] * A.m[1][0];
    out.m[0][1] = B.m[0][0] * A.m[0][1] + B.m[0][1] * A.m[1][1];
    out.m[1][0] = B.m[1][0] * A.m[0][0] + B.m[1][1] * A.m[1][0];
    out.m[1][1] = B.m[1][0] * A.m[0][1] + B.m[1][1] * A.m[1][1];

    // translation part (order of addition changed)
    out.m[0][2] = B.m[0][0] * A.m[0][2] + B.m[0][1] * A.m[1][2] + B.m[0][2];
    out.m[1][2] = B.m[1][0] * A.m[0][2] + B.m[1][1] * A.m[1][2] + B.m[1][2];
}

// -------------------------------------------------------------------
// Helper: apply a transform to an array of points
void applyToArray(const Aff2& mat, Pt2* src, Pt2* dst, int cnt)
{
    int i = 0;
    while (i < cnt) {
        dst[i] = transformPoint(mat, src[i]);
        ++i;
    }
}

// -------------------------------------------------------------------
int main()
{
    // deterministic set of test points (stack array)
    Pt2 srcPoints[7] = {
        { 0.0f, 0.0f },
        { 1.2f, -0.4f },
        { -0.7f, 2.3f },
        { 3.5f, 1.1f },
        { -2.0f, -1.5f },
        { 0.5f, 0.5f },
        { -1.3f, 0.8f }
    };
    const int N = 7;

    // first transform: rotate 57° and translate (3.5, -0.8)
    Aff2 t1;
    buildTransform(t1, 57.0f, 3.5f, -0.8f);

    // second transform: rotate 120° and translate (-2.2, 4.1)
    Aff2 t2;
    buildTransform(t2, 120.0f, -2.2f, 4.1f);

    // combine them (tCombined = t2 ∘ t1)
    Aff2 tCombined;
    combine(t1, t2, tCombined);

    // output buffers (stack)
    Pt2 out1[7];
    Pt2 out2[7];
    Pt2 outCombined[7];

    // apply each transform
    applyToArray(t1, srcPoints, out1, N);
    applyToArray(t2, srcPoints, out2, N);
    applyToArray(tCombined, srcPoints, outCombined, N);

    // print results
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "Original points:\n";
    for (int i = 0; i < N; ++i)
        std::cout << "  (" << srcPoints[i].x << ", " << srcPoints[i].y << ")\n";

    std::cout << "\nAfter Transform 1 (57° rot, +3.5,-0.8):\n";
    for (int i = 0; i < N; ++i)
        std::cout << "  (" << out1[i].x << ", " << out1[i].y << ")\n";

    std::cout << "\nAfter Transform 2 (120° rot, -2.2,+4.1):\n";
    for (int i = 0; i < N; ++i)
        std::cout << "  (" << out2[i].x << ", " << out2[i].y << ")\n";

    std::cout << "\nAfter Combined Transform (t2 ∘ t1):\n";
    for (int i = 0; i < N; ++i)
        std::cout << "  (" << outCombined[i].x << ", " << outCombined[i].y << ")\n";

    return 0;
}
