#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>

/* LLM input variant 2: small-diverse */

// ------------------------------------------------------------
// Helper: fill a float array with random values in [0,100)
// ------------------------------------------------------------
void fill_random(float* dst, int cnt)
{
    for (int i = 0; i < cnt; ++i)
    {
        dst[i] = (float)rand() / (float)RAND_MAX * 100.0f;
    }
}

// ------------------------------------------------------------
// Helper: compute the angle at vertex B of triangle ABC
// ------------------------------------------------------------
float angle_at_B(float ax, float ay,
                float bx, float by,
                float cx, float cy)
{
    // vectors BA and BC
    float v1x = ax - bx;
    float v1y = ay - by;
    float v2x = cx - bx;
    float v2y = cy - by;

    // dot product and lengths (order deliberately shuffled)
    float dot   = v1x * v2x + v1y * v2y;
    float len1  = sqrtf(v1x * v1x + v1y * v1y);
    float len2  = sqrtf(v2x * v2x + v2y * v2y);
    float denom = len1 * len2;

    // protect against division by zero
    if (denom == 0.0f) return 0.0f;

    float cosTheta = dot / denom;
    // clamp because of floating‑point noise
    if (cosTheta > 1.0f)  cosTheta = 1.0f;
    if (cosTheta < -1.0f) cosTheta = -1.0f;

    // final angle (radians)
    return acosf(cosTheta);
}

// ------------------------------------------------------------
int main()
{
    std::srand((unsigned)std::time(0));

    // size of the test set (small, diverse)
    int N = 6;

    // heap allocation for coordinates (x,y) of three points
    float* aX = new float[N];
    float* aY = new float[N];
    float* bX = new float[N];
    float* bY = new float[N];
    float* cX = new float[N];
    float* cY = new float[N];
    float* ang = new float[N];

    // deterministic diverse test data
    const float aX_vals[6] = {0.0f, 1.0f, -2.0f, 3.0f, 5.0f, 2.5f};
    const float aY_vals[6] = {0.0f, 0.0f,  1.0f, 4.0f, 0.0f, -1.0f};
    const float bX_vals[6] = {1.0f, 1.0f,  0.0f, 3.0f, 5.0f, 2.5f};
    const float bY_vals[6] = {0.0f, 0.0f,  0.0f, 0.0f, 3.0f, -1.0f};
    const float cX_vals[6] = {0.0f, 2.0f,  2.0f, 6.0f, 5.0f, 4.0f};
    const float cY_vals[6] = {1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 3.0f};

    for (int i = 0; i < N; ++i)
    {
        aX[i] = aX_vals[i];
        aY[i] = aY_vals[i];
        bX[i] = bX_vals[i];
        bY[i] = bY_vals[i];
        cX[i] = cX_vals[i];
        cY[i] = cY_vals[i];
    }

    // --------------------------------------------------------
    // Compute angles with manual loop unrolling (process 2 at a time)
    // --------------------------------------------------------
    int i = 0;
    while (i + 1 < N)
    {
        // first element
        ang[i] = angle_at_B(aX[i], aY[i],
                            bX[i], bY[i],
                            cX[i], cY[i]);

        // second element (i+1)
        ang[i + 1] = angle_at_B(aX[i + 1], aY[i + 1],
                                bX[i + 1], bY[i + 1],
                                cX[i + 1], cY[i + 1]);

        i += 2;
    }
    // handle possible trailing element
    if (i < N)
    {
        ang[i] = angle_at_B(aX[i], aY[i],
                            bX[i], bY[i],
                            cX[i], cY[i]);
    }

    // --------------------------------------------------------
    // Print the first few results as a sanity check
    // --------------------------------------------------------
    std::cout << "First 5 angles (radians):\n";
    for (int k = 0; k < 5 && k < N; ++k)
    {
        std::cout << ang[k] << "\n";
    }

    // clean up
    delete[] aX;
    delete[] aY;
    delete[] bX;
    delete[] bY;
    delete[] cX;
    delete[] cY;
    delete[] ang;

    return 0;
}
