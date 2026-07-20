#include <iostream>
#include <cmath>

/* LLM input variant 7: reverse-adversarial */

// ------------------------------------------------------------
// Helper: fill arrays with deterministic reverse‑ordered data
// ------------------------------------------------------------
void fill_deterministic(float* aX, float* aY,
                        float* bX, float* bY,
                        float* cX, float* cY,
                        int N)
{
    for (int i = 0; i < N; ++i)
    {
        // A moves from large positive X down to small positive X
        aX[i] = static_cast<float>(N - i);
        aY[i] = 0.0f;

        // B stays at origin (common vertex)
        bX[i] = 0.0f;
        bY[i] = 0.0f;

        // C moves from large negative X up to small negative X
        cX[i] = static_cast<float>(i - N);
        cY[i] = 0.0f;
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
    int N = 100000;

    // heap allocation for coordinates (x,y) of three points
    float* aX = new float[N];
    float* aY = new float[N];
    float* bX = new float[N];
    float* bY = new float[N];
    float* cX = new float[N];
    float* cY = new float[N];
    float* ang = new float[N];

    // fill arrays with deterministic reverse‑ordered coordinates
    fill_deterministic(aX, aY, bX, bY, cX, cY, N);

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
