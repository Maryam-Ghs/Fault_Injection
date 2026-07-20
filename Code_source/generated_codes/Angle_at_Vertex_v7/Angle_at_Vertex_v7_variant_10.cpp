#include <iostream>
#include <cmath>

/* LLM input variant 10: large-safe-stress */

class VertexAngle
{
public:
    float evaluate(const float pts[3][2])
    {
        float vec1[2];
        float vec2[2];

        for (int i = 0; i < 2; ++i)
        {
            vec1[i] = pts[0][i] - pts[1][i];
            vec2[i] = pts[2][i] - pts[1][i];
        }

        float dot = 0.0f;
        for (int i = 0; i < 2; ++i)
            dot += vec1[i] * vec2[i];

        float sq1 = 0.0f, sq2 = 0.0f;
        for (int i = 0; i < 2; ++i)
        {
            sq1 += vec1[i] * vec1[i];
            sq2 += vec2[i] * vec2[i];
        }

        if (sq1 == 0.0f || sq2 == 0.0f)
            return -1.0f;

        float len1 = std::sqrt(sq1);
        float len2 = std::sqrt(sq2);

        float cosTheta = dot / (len1 * len2);
        if (cosTheta > 1.0f) cosTheta = 1.0f;
        if (cosTheta < -1.0f) cosTheta = -1.0f;

        float angleRad = std::acos(cosTheta);
        float angleDeg = angleRad * (180.0f / 3.14159265358979323846f);
        return angleDeg;
    }
};

int main()
{
    const int CASES = 1000;
    float data[CASES][3][2];

    for (int c = 0; c < CASES; ++c)
    {
        // Generate diverse test cases based on the case index
        if (c % 4 == 0)
        {
            // Regular triangle pattern
            data[c][0][0] = static_cast<float>(c);
            data[c][0][1] = 0.0f;
            data[c][1][0] = static_cast<float>(c + 1);
            data[c][1][1] = 0.0f;
            data[c][2][0] = static_cast<float>(c + 1);
            data[c][2][1] = 1.0f;
        }
        else if (c % 4 == 1)
        {
            // Colinear points (straight line)
            data[c][0][0] = static_cast<float>(c);
            data[c][0][1] = static_cast<float>(c);
            data[c][1][0] = static_cast<float>(c + 2);
            data[c][1][1] = static_cast<float>(c + 2);
            data[c][2][0] = static_cast<float>(c + 4);
            data[c][2][1] = static_cast<float>(c + 4);
        }
        else if (c % 4 == 2)
        {
            // Zero‑length side (B coincides with A)
            data[c][0][0] = static_cast<float>(c);
            data[c][0][1] = static_cast<float>(c);
            data[c][1][0] = static_cast<float>(c);
            data[c][1][1] = static_cast<float>(c);
            data[c][2][0] = static_cast<float>(c + 3);
            data[c][2][1] = static_cast<float>(c + 5);
        }
        else // c % 4 == 3
        {
            // Obtuse angle configuration
            data[c][0][0] = 0.0f;
            data[c][0][1] = 0.0f;
            data[c][1][0] = static_cast<float>(c % 50 + 1);
            data[c][1][1] = 0.0f;
            data[c][2][0] = static_cast<float>(c % 50 + 2);
            data[c][2][1] = -1.0f;
        }
    }

    VertexAngle solver;

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
