#include <iostream>
#include <cstdlib>
#include <ctime>

#define POINT_COUNT 1024   // size of the point array

/* LLM input variant 5: duplicate-heavy */

int main()
{
    // ------------------------------------------------------------
    // 1. Initialise random number generator (seed with a fixed value for reproducibility)
    // ------------------------------------------------------------
    int seed = 12345;               // fixed seed
    srand(seed);                    // set the seed for rand()

    // ------------------------------------------------------------
    // 2. Allocate stack arrays for input points and output points
    // ------------------------------------------------------------
    float srcX[POINT_COUNT];
    float srcY[POINT_COUNT];
    float dstX[POINT_COUNT];
    float dstY[POINT_COUNT];

    // ------------------------------------------------------------
    // 3. Fill the source arrays with a repeating pattern of duplicate coordinates
    // ------------------------------------------------------------
    for (int i = 0; i < POINT_COUNT; ++i)
    {
        // Create a small set of distinct values that repeat often
        switch (i % 8)
        {
            case 0: srcX[i] =  50.0f; srcY[i] =  50.0f; break;
            case 1: srcX[i] = -50.0f; srcY[i] =  50.0f; break;
            case 2: srcX[i] =  50.0f; srcY[i] = -50.0f; break;
            case 3: srcX[i] = -50.0f; srcY[i] = -50.0f; break;
            case 4: srcX[i] =  0.0f;  srcY[i] =  0.0f;  break;
            case 5: srcX[i] =  0.0f;  srcY[i] =  0.0f;  break;
            case 6: srcX[i] =  0.0f;  srcY[i] =  0.0f;  break;
            case 7: srcX[i] =  0.0f;  srcY[i] =  0.0f;  break;
        }
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
    // ------------------------------------------------------------
    for (int i = 0; i < POINT_COUNT; ++i)
    {
        std::cout << "P" << i << ": ("
                  << srcX[i] << ", " << srcY[i] << ")  -->  ("
                  << dstX[i] << ", " << dstY[i] << ")\n";
    }

    return 0;
}
