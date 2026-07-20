#include <iostream>
#include <cmath>
#include <cstdlib>

/* LLM input variant 8: sparse-skewed */

// ------------------------------------------------------------
// Helper: fill a float array with sparse, skewed values
// Most entries are zero; every 1000th entry gets a moderate value,
// creating a highly imbalanced, clustered pattern.
// ------------------------------------------------------------
void fill_random(float* dst, int cnt)
{
    for (int i = 0; i < cnt; ++i)
    {
        if (i % 1000 == 0)
        {
            // Clustered non‑zero values, slightly varying
            dst[i] = 45.0f + static_cast<float>((i / 1000) % 10);
        }
        else
        {
            // Majority of entries are zero (sparse)
            dst[i] = 0.0f;
        }
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
    // size of the test set (large array)
    int N = 20000;   // reduced size but still demonstrates sparsity

    // heap allocation for coordinates (x,y) of three points
    float* aX = new float[N];
    float* aY = new float[N];
    float* bX = new float[N];
    float* bY = new float[N];
    float* cX = new float[N];
    float* cY = new float[N];
    float* ang = new float[N];

    // fill arrays with sparse, skewed coordinates
    fill_random(aX, N);
    fill_random(aY, N);
    fill_random(bX, N);
    fill_random(bY, N);
    fill_random(cX, N);
    fill_random(cY, N);

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
