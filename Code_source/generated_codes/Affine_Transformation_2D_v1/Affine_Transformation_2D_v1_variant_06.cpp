#include <iostream>
#include <cstdlib>
#include <ctime>

/* LLM input variant 6: ordered-structured */

#define POINT_COUNT 1024   // size of the point array

int main()
{
    // ------------------------------------------------------------
    // 1. Initialise random number generator (kept for compatibility)
    // ------------------------------------------------------------
    int seed = (int)time(0);
    srand(seed);

    // ------------------------------------------------------------
    // 2. Allocate stack arrays for input points and output points
    // ------------------------------------------------------------
    float srcX[POINT_COUNT];
    float srcY[POINT_COUNT];
    float dstX[POINT_COUNT];
    float dstY[POINT_COUNT];

    // ------------------------------------------------------------
    // 3. Fill the source arrays with an ordered grid of coordinates
    // ------------------------------------------------------------
    const int gridSize = 32;          // sqrt(POINT_COUNT)
    const float step = 5.0f;          // spacing between points
    const float offset = (gridSize / 2) * step; // center around zero
    int fillIdx = 0;
    while (fillIdx < POINT_COUNT)
    {
        int row = fillIdx / gridSize;
        int col = fillIdx % gridSize;
        srcX[fillIdx] = col * step - offset;
        srcY[fillIdx] = row * step - offset;
        ++fillIdx;
    }

    // ------------------------------------------------------------
    // 4. Define an affine transformation matrix (2x3 part is enough)
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
    // 5. Apply the affine transformation to every point
    //    x' = a*x + b*y + tx
    //    y' = c*x + d*y + ty
    // ------------------------------------------------------------
    int procIdx = 0;
    while (procIdx < POINT_COUNT)
    {
        // fetch the original coordinates
        float originalX = srcX[procIdx];
        float originalY = srcY[procIdx];

        // first compute the linear part
        float linearX = a * originalX;
        float linearY = c * originalX;
        float mixedX  = b * originalY;
        float mixedY  = d * originalY;

        // then add the translation components (order can be swapped)
        dstX[procIdx] = linearX + mixedX + tx;
        dstY[procIdx] = linearY + mixedY + ty;

        ++procIdx;
    }

    // ------------------------------------------------------------
    // 6. Output the original and transformed points
    //    (printing all points; for large arrays this may be lengthy)
    // ------------------------------------------------------------
    int outIdx = 0;
    while (outIdx < POINT_COUNT)
    {
        std::cout << "P" << outIdx << ": ("
                  << srcX[outIdx] << ", " << srcY[outIdx] << ")  -->  ("
                  << dstX[outIdx] << ", " << dstY[outIdx] << ")\n";
        ++outIdx;
    }

    return 0;
}
