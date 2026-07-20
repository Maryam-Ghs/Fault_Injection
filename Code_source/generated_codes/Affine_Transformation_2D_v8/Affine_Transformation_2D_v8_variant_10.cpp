#include <iostream>
#include <iomanip>
#include <vector>

/* LLM input variant 10: large-safe-stress */

int main()
{
    // version #8 ---------------------------------------------------------

    // ----- test data (deterministic) ------------------------------------
    const int pointCount = 1000;  // larger safe dataset

    // struct for points
    struct Pt { float x; float y; };

    // generate a deterministic set of source points
    std::vector<Pt> src(pointCount);
    for (int i = 0; i < pointCount; ++i) {
        src[i].x = static_cast<float>(i % 100);          // x cycles every 100
        src[i].y = static_cast<float>(i / 100);         // y increments every 100 points
    }

    // affine matrix stored as two rows, three columns:
    // [ a  b  tx ]
    // [ c  d  ty ]
    float aff[2][3] = {
        {  0.8f, -0.6f,  5.0f },   // row 0 : a, b, tx
        {  0.6f,  0.8f, -3.0f }    // row 1 : c, d, ty
    };

    // vector for transformed points
    std::vector<Pt> dst(pointCount);

    // ----- apply transformation -----------------------------------------
    for (int idx = 0; idx < pointCount; ++idx)
    {
        // reorder arithmetic while keeping mathematical meaning
        float txPart = aff[0][2] + aff[0][0] * src[idx].x;
        float xNew   = txPart + aff[0][1] * src[idx].y;

        float tyPart = aff[1][2] + aff[1][0] * src[idx].x;
        float yNew   = tyPart + aff[1][1] * src[idx].y;

        dst[idx].x = xNew;
        dst[idx].y = yNew;
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
