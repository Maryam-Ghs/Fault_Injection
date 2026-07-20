#include <iostream>
#include <cmath>

/* LLM input variant 8: sparse-skewed */

class VertexAngle
{
public:
    // Compute the angle (in degrees) at vertex B formed by points A-B-C
    // pts[0] = A, pts[1] = B, pts[2] = C ; each point has 2 coordinates (x,y)
    float evaluate(const float pts[3][2])
    {
        // Vectors BA and BC
        float vec1[2];
        float vec2[2];

        // Build vectors using a loop (loop‑heavy)
        for (int i = 0; i < 2; ++i)
        {
            vec1[i] = pts[0][i] - pts[1][i]; // A - B
            vec2[i] = pts[2][i] - pts[1][i]; // C - B
        }

        // Dot product (loop‑heavy)
        float dot = 0.0f;
        for (int i = 0; i < 2; ++i)
            dot += vec1[i] * vec2[i];

        // Lengths (re‑ordered: compute squares first)
        float sq1 = 0.0f, sq2 = 0.0f;
        for (int i = 0; i < 2; ++i)
        {
            sq1 += vec1[i] * vec1[i];
            sq2 += vec2[i] * vec2[i];
        }

        // Guard against zero‑length sides (edge‑case)
        if (sq1 == 0.0f || sq2 == 0.0f)
            return -1.0f; // sentinel for undefined angle

        float len1 = std::sqrt(sq1);
        float len2 = std::sqrt(sq2);

        // Cosine of the angle
        float cosTheta = dot / (len1 * len2);

        // Clamp due to floating point rounding errors
        if (cosTheta > 1.0f) cosTheta = 1.0f;
        if (cosTheta < -1.0f) cosTheta = -1.0f;

        // Angle in degrees (standard direct computation)
        float angleRad = std::acos(cosTheta);
        float angleDeg = angleRad * (180.0f / 3.14159265358979323846f);
        return angleDeg;
    }
};

int main()
{
    // Stack‑allocated array of test cases (sparse‑skewed input)
    // Each case: 3 points * 2 coordinates
    const int CASES = 8;
    float data[CASES][3][2] = {
        // Regular small triangle
        { {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f} },
        // Zero‑length side (B coincides with A) with many zeros
        { {0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 1000.0f} },
        // Extreme coordinates (large magnitude, sparse non‑zero)
        { {1e6f, -1e6f}, {0.0f, 0.0f}, {-1e6f, 1e6f} },
        // Clustered near‑zero points, colinear
        { {0.001f, 0.001f}, {0.002f, 0.002f}, {0.003f, 0.003f} },
        // Skewed almost straight line with a distant point
        { {0.0f, 0.0f}, {1.0f, 0.0f}, {1000.0f, 0.001f} },
        // Zero‑length side (B coincides with C)
        { {3.0f, 4.0f}, {5.0f, 6.0f}, {5.0f, 6.0f} },
        // All points identical (completely empty/zero‑length)
        { {0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f} },
        // Small angle due to tiny offset
        { {5.0f, 5.0f}, {5.0f, 5.000001f}, {5.0f, 10.0f} }
    };

    VertexAngle solver;

    // Iterate over all cases (loop‑heavy)
    for (int c = 0; c < CASES; ++c)
    {
        float result = solver.evaluate(data[c]);

        std::cout << "Case " << c + 1 << ": ";
        if (result < 0.0f)
            std::cout << "Undefined (zero‑length side)";
        else
            std::cout << "Angle = " << result << " degrees";
        std::cout << std::endl;
    }

    return 0;
}
