#include <iostream>
#include <iomanip>

/* LLM input variant 9: medium-deterministic-random */

int main()
{
    // version #8 ---------------------------------------------------------

    // ----- test data (deterministic) ------------------------------------
    int pointCount = 4;

    // stack array of original points (x, y)
    struct Pt { float x; float y; };
    Pt src[4] = {
        { -5.0f,  3.0f },
        { 12.5f, -7.0f },
        {  0.0f, 15.0f },
        {  8.0f,  8.0f }
    };

    // affine matrix stored as two rows, three columns:
    // [ a  b  tx ]
    // [ c  d  ty ]
    float aff[2][3] = {
        {  0.5f,  0.866f, -2.0f },   // row 0 : a, b, tx
        { -0.866f, 0.5f,  4.5f }    // row 1 : c, d, ty
    };

    // stack array for transformed points
    Pt dst[4];

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
