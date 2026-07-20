// LLM input variant 10: large-safe-stress
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
    // larger deterministic set of test points (stack array)
    const int N = 1000;
    Pt2 srcPoints[N];
    for (int i = 0; i < N; ++i) {
        // create a grid-like pattern within a reasonable range
        srcPoints[i].x = static_cast<float>((i % 100) - 50) * 0.1f;   // -5.0 .. 4.9
        srcPoints[i].y = static_cast<float>((i / 100) - 5) * 0.1f;   // -5.0 .. 4.9
    }

    // first transform: rotate 30° and translate (2, 1)
    Aff2 t1;
    buildTransform(t1, 30.0f, 2.0f, 1.0f);

    // second transform: rotate -45° and translate (-1, 0.5)
    Aff2 t2;
    buildTransform(t2, -45.0f, -1.0f, 0.5f);

    // combine them (tCombined = t2 ∘ t1)
    Aff2 tCombined;
    combine(t1, t2, tCombined);

    // output buffers (stack)
    Pt2 out1[N];
    Pt2 out2[N];
    Pt2 outCombined[N];

    // apply each transform
    applyToArray(t1, srcPoints, out1, N);
    applyToArray(t2, srcPoints, out2, N);
    applyToArray(tCombined, srcPoints, outCombined, N);

    // print a subset to keep output manageable
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "Original points (first 10):\n";
    for (int i = 0; i < 10; ++i)
        std::cout << "  (" << srcPoints[i].x << ", " << srcPoints[i].y << ")\n";

    std::cout << "\nAfter Transform 1 (30° rot, +2,+1) (first 10):\n";
    for (int i = 0; i < 10; ++i)
        std::cout << "  (" << out1[i].x << ", " << out1[i].y << ")\n";

    std::cout << "\nAfter Transform 2 (-45° rot, -1,+0.5) (first 10):\n";
    for (int i = 0; i < 10; ++i)
        std::cout << "  (" << out2[i].x << ", " << out2[i].y << ")\n";

    std::cout << "\nAfter Combined Transform (t2 ∘ t1) (first 10):\n";
    for (int i = 0; i < 10; ++i)
        std::cout << "  (" << outCombined[i].x << ", " << outCombined[i].y << ")\n";

    return 0;
}
