#include <iostream>

/* LLM input variant 3: zeros-and-ones */

#define POINT_COUNT 8   // small deterministic dataset emphasizing zeros and ones

int main()
{
    // ------------------------------------------------------------
    // 1. Allocate stack arrays for input points and output points
    // ------------------------------------------------------------
    float srcX[POINT_COUNT];
    float srcY[POINT_COUNT];
    float dstX[POINT_COUNT];
    float dstY[POINT_COUNT];

    // ------------------------------------------------------------
    // 2. Fill the source arrays with deterministic zeros and ones
    // ------------------------------------------------------------
    for (int i = 0; i < POINT_COUNT; ++i)
    {
        // Alternate between 0.0 and 1.0 for both coordinates
        srcX[i] = (i % 2 == 0) ? 0.0f : 1.0f;
        srcY[i] = (i % 2 == 0) ? 1.0f : 0.0f;
    }

    // ------------------------------------------------------------
    // 3. Define an identity affine transformation matrix
    //    | a  b  tx |
    //    | c  d  ty |
    // ------------------------------------------------------------
    float a  = 1.0f;   // identity linear part X scale
    float b  = 0.0f;   // identity linear part X shear
    float c  = 0.0f;   // identity linear part Y shear
    float d  = 1.0f;   // identity linear part Y scale
    float tx = 0.0f;   // no translation in X
    float ty = 0.0f;   // no translation in Y

    // ------------------------------------------------------------
    // 4. Apply the affine transformation to every point
    //    x' = a*x + b*y + tx
    //    y' = c*x + d*y + ty
    // ------------------------------------------------------------
    for (int i = 0; i < POINT_COUNT; ++i)
    {
        float originalX = srcX[i];
        float originalY = srcY[i];

        float linearX = a * originalX;
        float linearY = c * originalX;
        float mixedX  = b * originalY;
        float mixedY  = d * originalY;

        dstX[i] = linearX + mixedX + tx;
        dstY[i] = linearY + mixedY + ty;
    }

    // ------------------------------------------------------------
    // 5. Output the original and transformed points
    // ------------------------------------------------------------
    for (int i = 0; i < POINT_COUNT; ++i)
    {
        std::cout << "P" << i << ": ("
                  << srcX[i] << ", " << srcY[i] << ")  -->  ("
                  << dstX[i] << ", " << dstY[i] << ")\n";
    }

    return 0;
}
