/* LLM input variant 9: medium-deterministic-random */
// Angle Between Two Vectors – Version 9
// -------------------------------------------------
// Generates two large deterministic pseudo‑random vectors, computes the angle
// between them using manual loop unrolling and fused arithmetic expressions.
// Only float and int are used.

#include <iostream>
#include <vector>
#include <cmath>
#include <cstdint>

// -----------------------------------------------------------------
// Deterministic pseudo‑random generator (LCG)
static uint32_t lcg_state = 0xDEADBEEFu;
static uint32_t lcg_next()
{
    // Constants from Numerical Recipes
    lcg_state = lcg_state * 1664525u + 1013904223u;
    return lcg_state;
}

// Helper: fill a vector with deterministic numbers in the range [-100,100]
std::vector<float> build_deterministic_vec(int sz)
{
    std::vector<float> vec;
    vec.reserve(sz);
    for (int i = 0; i < sz; ++i)
    {
        uint32_t rnd = lcg_next();                     // 0 .. 2^32‑1
        int   val = static_cast<int>(rnd % 201) - 100; // -100 .. 100
        vec.push_back(static_cast<float>(val));
    }
    return vec;
}

// -----------------------------------------------------------------
// Helper: compute dot product and squared lengths in one pass,
// using manual loop unrolling (4 elements per iteration)
void gather_stats(std::vector<float>& v1,
                  std::vector<float>& v2,
                  float& dotProd,
                  float& len1Sq,
                  float& len2Sq)
{
    dotProd = 0.0f;
    len1Sq = 0.0f;
    len2Sq = 0.0f;

    int idx = 0;
    int total = static_cast<int>(v1.size());

    // main unrolled loop
    while (idx + 3 < total)
    {
        float a0 = v1[idx];
        float b0 = v2[idx];
        float a1 = v1[idx + 1];
        float b1 = v2[idx + 1];
        float a2 = v1[idx + 2];
        float b2 = v2[idx + 2];
        float a3 = v1[idx + 3];
        float b3 = v2[idx + 3];

        // fused expression: update all three accumulators together
        dotProd += a0 * b0 + a1 * b1 + a2 * b2 + a3 * b3;
        len1Sq += a0 * a0 + a1 * a1 + a2 * a2 + a3 * a3;
        len2Sq += b0 * b0 + b1 * b1 + b2 * b2 + b3 * b3;

        idx += 4;
    }

    // tail loop for remaining elements
    while (idx < total)
    {
        float a = v1[idx];
        float b = v2[idx];
        dotProd += a * b;
        len1Sq += a * a;
        len2Sq += b * b;
        ++idx;
    }
}

// -----------------------------------------------------------------
// Helper: compute the angle (in radians) between two vectors
float calc_angle(std::vector<float>& vecA, std::vector<float>& vecB)
{
    float dotAB, magASq, magBSq;
    gather_stats(vecA, vecB, dotAB, magASq, magBSq);

    // reorder: compute product of magnitudes first
    float magProd = std::sqrt(magASq) * std::sqrt(magBSq);

    // guard against division by zero
    if (magProd == 0.0f)
        return 0.0f;

    // fused division and acos
    float ratio = dotAB / magProd;
    // clamp ratio into [-1,1] to avoid NaN from floating‑point errors
    if (ratio > 1.0f)  ratio = 1.0f;
    if (ratio < -1.0f) ratio = -1.0f;

    return static_cast<float>(std::acos(ratio));
}

// -----------------------------------------------------------------
int main()
{
    // deterministic pseudo‑random vectors (e.g., 10,000 elements each)
    int dim = 10000;
    std::vector<float> first  = build_deterministic_vec(dim);
    std::vector<float> second = build_deterministic_vec(dim);

    float radAngle = calc_angle(first, second);
    float degAngle = radAngle * (180.0f / 3.14159265f);

    std::cout << "Angle between vectors (radians): " << radAngle << '\n';
    std::cout << "Angle between vectors (degrees): " << degAngle << '\n';

    return 0;
}
