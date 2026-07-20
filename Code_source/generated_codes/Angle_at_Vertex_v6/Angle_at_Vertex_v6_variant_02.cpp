#include <iostream>
#include <vector>
#include <cmath>

/* LLM input variant 2: small-diverse */

struct Point
{
    float x;
    float y;
};

class VertexAngle
{
public:
    VertexAngle() {}

    // generate a small deterministic set of points
    void produce_points(int amount)
    {
        // predefined diverse points
        static const Point preset[] = {
            {0.0f, 0.0f},
            {1.0f, 0.0f},
            {0.0f, 1.0f},
            {-1.0f, 0.0f},
            {0.0f, -1.0f},
            {2.5f, 2.5f},
            {-2.0f, -3.0f}
        };
        int presetSize = sizeof(preset) / sizeof(preset[0]);
        for (int i = 0; i < amount && i < presetSize; ++i)
        {
            storage.push_back(preset[i]);
        }
    }

    // compute the angle at the middle point of every consecutive triple
    void evaluate()
    {
        int total = static_cast<int>(storage.size());
        // we need at least three points for one angle
        int limit = total - 2;

        for (int i = 0; i < limit; ++i)
        {
            // take three successive points
            Point a = storage[i];
            Point b = storage[i + 1];
            Point c = storage[i + 2];

            // vectors BA and BC
            float vx1 = a.x - b.x;
            float vy1 = a.y - b.y;
            float vx2 = c.x - b.x;
            float vy2 = c.y - b.y;

            // dot product
            float dot = vx1 * vx2 + vy1 * vy2;

            // lengths (magnitudes)
            float len1 = std::sqrt(vx1 * vx1 + vy1 * vy1);
            float len2 = std::sqrt(vx2 * vx2 + vy2 * vy2);

            // avoid division by zero
            if (len1 == 0.0f || len2 == 0.0f)
            {
                std::cout << "0\n";
                continue;
            }

            // cosine of the angle
            float cos_val = dot / (len1 * len2);

            // clamp because of possible rounding errors
            if (cos_val > 1.0f) cos_val = 1.0f;
            if (cos_val < -1.0f) cos_val = -1.0f;

            // angle in radians then degrees
            float rad = std::acos(cos_val);
            float deg = rad * (180.0f / 3.1415926f);

            // output the result
            std::cout << deg << '\n';
        }
    }

private:
    std::vector<Point> storage;
};

int main()
{
    VertexAngle solver;
    // generate 7 deterministic points (will produce 5 angles)
    solver.produce_points(7);
    solver.evaluate();
    return 0;
}
