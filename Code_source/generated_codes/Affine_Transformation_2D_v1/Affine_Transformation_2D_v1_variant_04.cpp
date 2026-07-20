#include <iostream>
#include <cstdlib>
#include <ctime>

#define POINT_COUNT 1024   // size of the random point array

/* LLM input variant 4: signed-extremes */

int main()
{
    // ------------------------------------------------------------
    // 1. Initialise random number generator (seed with current time)
    // ------------------------------------------------------------
    int seed = (int)time(0);          // time() returns time_t (long), cast to int
    srand(seed);                     // set the seed for rand()

    // ------------------------------------------------------------
    // 2. Allocate stack arrays for input points and output points
    // ------------------------------------------------------------
    float srcX[POINT_COUNT];
    float srcY[POINT_COUNT];
    float dstX[POINT_COUNT];
    float dstY[POINT_COUNT];

    // ------------------------------------------------------------
    // 3. Fill the source arrays with deterministic coordinates mixing
    //    negative, zero, and positive extremes
    // ------------------------------------------------------------
    for (int i = 0; i < POINT_COUNT; ++i)
    {
        // Cycle through -100, 0, 100
        int mod = i % 3;               // 0,1,2
        float value = (mod - 1) * 100.0f; // -100, 0, 100

        srcX[i] = value;
        srcY[i] = -value; // opposite sign to add variety
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
    // 6. Output the original and transformed points
    //    (printing all points; for large arrays this may be lengthy)
    // ------------------------------------------------------------
    for (int i = 0; i < POINT_COUNT; ++i)
    {
        std::cout << "P" << i << ": ("
                  << srcX[i] << ", " << srcY[i] << ")  -->  ("
                  << dstX[i] << ", " << dstY[i] << ")\n";
    }

    return 0;
}
