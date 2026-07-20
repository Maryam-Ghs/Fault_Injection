/* LLM input variant 8: sparse-skewed */
// Angle Between Two Vectors – Version 9
// -------------------------------------------------
// Generates two large random vectors, computes the angle
// between them using manual loop unrolling and fused
// arithmetic expressions.  Only float and int are used.

#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>

// -----------------------------------------------------------------
// Helper: fill a vector with mostly zeros, occasional clustered non‑zeros
std::vector<float> build_random_vec(int sz)
{
    std::vector<float> vec;
    vec.reserve(sz);
    for (int i = 0; i < sz; ++i)
    {
        // Create two small clusters of non‑zero values per 5000 elements
        if (i % 5000 < 10)   // first 10 positions of each 5000‑block
        {
            // rand() yields int; cast to float after shifting to signed range
            vec.push_back(static_cast<float>((std::rand() % 201) - 100));
        }
        else
        {
            vec.push_back(0.0f);
        }
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
    // Fixed seed for deterministic sparse input
    std::srand(42);

    // large random arrays (e.g., 10,000 elements each) now mostly sparse
    int dim = 10000;
    std::vector<float> first  = build_random_vec(dim);
    std::vector<float> second = build_random_vec(dim);

    float radAngle = calc_angle(first, second);
    float degAngle = radAngle * (180.0f / 3.14159265f);

    std::cout << "Angle between vectors (radians): " << radAngle << '\n';
    std::cout << "Angle between vectors (degrees): " << degAngle << '\n';

    return 0;
}
