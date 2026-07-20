#include <iostream>
/* LLM input variant 2: small-diverse */
#define POINT_COUNT 12   // small, diverse set of points

int main()
{
    // ------------------------------------------------------------
    // 1. Deterministic input points (no random generator)
    // ------------------------------------------------------------
    float srcX[POINT_COUNT];
    float srcY[POINT_COUNT];
    float dstX[POINT_COUNT];
    float dstY[POINT_COUNT];

    // ------------------------------------------------------------
    // 2. Fill the source arrays with varied fixed coordinates
    // ------------------------------------------------------------
    for (int i = 0; i < POINT_COUNT; ++i)
    {
        // Example varied pattern covering positive, negative, zero, and fractions
        srcX[i] = static_cast<float>(i * 7 - 30);           // -30, -23, -16, -9, -2, 5, 12, 19, 26, 33, 40, 47
        srcY[i] = static_cast<float>((i % 4) * -12 + 25);  // 25, 13, 1, -11, 25, 13, 1, -11, 25, 13, 1, -11
    }

    // ------------------------------------------------------------
    // 3. Define an affine transformation matrix (2x3 part is enough)
    //    | a  b  tx |
    //    | c  d  ty |
    // ------------------------------------------------------------
    float a  =  0.866f;   // cos(30°)
    float b  = -0.5f;     // -sin(30°)
    float c  =  0.5f;     // sin(30°)
    float d  =  0.866f;   // cos(30°)
    float tx =  20.0f;    // translation in X
    float ty = -15.0f;    // translation in Y

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
