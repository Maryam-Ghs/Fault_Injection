/* LLM input variant 1: minimal-boundary */
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>

struct Point
{
    float x;
    float y;
};

class VertexAngle
{
public:
    VertexAngle() {}

    // generate a deterministic set of points
    void produce_points(int amount)
    {
        std::srand(42); // fixed seed for reproducibility
        for (int idx = 0; idx < amount; ++idx)
        {
            Point pt;
            pt.x = static_cast<float>(std::rand() % 200 - 100); // -100 .. 99
            pt.y = static_cast<float>(std::rand() % 200 - 100);
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
    // generate the minimal number of points (3) to produce exactly one angle
    solver.produce_points(3);
    solver.evaluate();
    return 0;
}
