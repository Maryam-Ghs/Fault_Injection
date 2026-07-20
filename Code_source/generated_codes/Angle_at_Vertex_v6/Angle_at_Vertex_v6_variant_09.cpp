#include <iostream>
#include <vector>
#include <cmath>

/* LLM input variant 9: medium-deterministic-random */

struct Point
{
    float x;
    float y;
};

class VertexAngle
{
public:
    VertexAngle() {}

    // generate a deterministic set of pseudo‑random points
    void produce_points(int amount)
    {
        unsigned int seed = 123456789u; // fixed seed for reproducibility
        for (int idx = 0; idx < amount; ++idx)
        {
            // simple linear congruential generator steps
            seed = seed * 1664525u + 1013904223u;
            int val_x = static_cast<int>(seed % 200); // 0 .. 199
            seed = seed * 1664525u + 1013904223u;
            int val_y = static_cast<int>(seed % 200);

            Point pt;
            pt.x = static_cast<float>(val_x - 100); // -100 .. 99
            pt.y = static_cast<float>(val_y - 100);
            storage.push_back(pt);
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
    // generate 5000 deterministic points (will produce 4998 angles)
    solver.produce_points(5000);
    solver.evaluate();
    return 0;
}
