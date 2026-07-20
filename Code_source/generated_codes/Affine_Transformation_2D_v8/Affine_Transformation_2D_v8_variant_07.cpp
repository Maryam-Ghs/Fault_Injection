/* LLM input variant 7: reverse-adversarial */
#include <iostream>
#include <iomanip>

int main()
{
    // version #8 ---------------------------------------------------------

    // ----- test data (deterministic) ------------------------------------
    int pointCount = 4;

    // stack array of original points (x, y) in reverse order and with extreme values
    struct Pt { float x; float y; };
    Pt src[4] = {
        {  0.0f, 20.0f },
        { 10.0f, 20.0f },
        { 10.0f,  0.0f },
        {  0.0f,  0.0f }
    };

    // affine matrix stored as two rows, three columns:
    // [ a  b  tx ]
    // [ c  d  ty ]
    // Using a reflection with large translation to create an adversarial scenario
    float aff[2][3] = {
        { -1.0f,  0.0f, 1000.0f },   // row 0 : a, b, tx
        {  0.0f, -1.0f, 2000.0f }    // row 1 : c, d, ty
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
