#include <iostream>
#include <cmath>
#include <random>

/* LLM input variant 10: large-safe-stress */

// ------------------------------------------------------------
// Helper: fill a float array with deterministic pseudo‑random values in [0,100)
// ------------------------------------------------------------
void fill_deterministic(float* dst, int cnt)
{
    std::mt19937 rng(123456789);               // fixed seed for reproducibility
    std::uniform_real_distribution<float> dist(0.0f, 100.0f);
    for (int i = 0; i < cnt; ++i)
    {
        dst[i] = dist(rng);
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
    // size of the test set (very large but memory‑safe)
    const int N = 1'000'000;   // one million entries

    // heap allocation for coordinates (x,y) of three points
    float* aX = new float[N];
    float* aY = new float[N];
    float* bX = new float[N];
    float* bY = new float[N];
    float* cX = new float[N];
    float* cY = new float[N];
    float* ang = new float[N];

    // fill arrays with deterministic pseudo‑random coordinates
    fill_deterministic(aX, N);
    fill_deterministic(aY, N);
    fill_deterministic(bX, N);
    fill_deterministic(bY, N);
    fill_deterministic(cX, N);
    fill_deterministic(cY, N);

    // --------------------------------------------------------
    // Compute angles with manual loop unrolling (process 4 at a time)
    // --------------------------------------------------------
    int i = 0;
    while (i + 3 < N)
    {
        ang[i]     = angle_at_B(aX[i], aY[i], bX[i], bY[i], cX[i], cY[i]);
        ang[i + 1] = angle_at_B(aX[i + 1], aY[i + 1], bX[i + 1], bY[i + 1], cX[i + 1], cY[i + 1]);
        ang[i + 2] = angle_at_B(aX[i + 2], aY[i + 2], bX[i + 2], bY[i + 2], cX[i + 2], cY[i + 2]);
        ang[i + 3] = angle_at_B(aX[i + 3], aY[i + 3], bX[i + 3], bY[i + 3], cX[i + 3], cY[i + 3]);
        i += 4;
    }
    // handle remaining elements
    while (i < N)
    {
        ang[i] = angle_at_B(aX[i], aY[i], bX[i], bY[i], cX[i], cY[i]);
        ++i;
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
