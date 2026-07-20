#include <iostream>
#include <iomanip>

/* LLM input variant 8: sparse-skewed */

int main()
{
    // version #8 ---------------------------------------------------------

    // ----- test data (deterministic) ------------------------------------
    int pointCount = 20;

    // stack array of original points (x, y)
    struct Pt { float x; float y; };
    Pt src[20] = {
        { 0.0f, 0.0f },
        { 0.0f, 0.0f },
        { 0.0f, 0.0f },
        { 1.0f, 1.0f },
        { 1.0f, 1.0f },
        { 1.0f, 1.0f },
        { 5.0f, 5.0f },
        { 5.0f, 5.0f },
        { 5.0f, 5.0f },
        {10.0f,10.0f },
        {10.0f,10.0f },
        {10.0f,10.0f },
        {50.0f,100.0f},
        {100.0f,200.0f},
        {200.0f,400.0f},
        { 0.0f,20.0f},
        { 0.0f,20.0f},
        { 0.0f,20.0f},
        { 0.0f, 0.0f},
        { 0.0f, 0.0f}
    };

    // affine matrix stored as two rows, three columns:
    // [ a  b  tx ]
    // [ c  d  ty ]
    float aff[2][3] = {
        { 2.0f, 0.0f, 5.0f },   // row 0 : a (scale x), b (shear), tx
        { 0.0f, 0.0f, 0.0f }    // row 1 : c, d (collapse y), ty
    };

    // stack array for transformed points
    Pt dst[20];

    // ----- apply transformation -----------------------------------------
    int idx = 0;
    while (idx < pointCount)
    {
        // reorder arithmetic while keeping mathematical meaning
        float txPart = aff[0][2] + aff[0][0] * src[idx].x;
        float xNew   = txPart + aff[0][1] * src[idx].y;

        float tyPart = aff[1][2] + aff[1][0] * src[idx].x;
        float yNew   = tyPart + aff[1][1] * src[idx].y;

        dst[idx].x = xNew;
        dst[idx].y = yNew;

        ++idx;
    }

    // ----- output ---------------------------------------------------------
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Original -> Transformed (Affine)\n";
    for (int i = 0; i < pointCount; ++i)
    {
        std::cout << "(" << src[i].x << ", " << src[i].y << ")  ->  "
                  << "(" << dst[i].x << ", " << dst[i].y << ")\n";
    }

    return 0;
}
