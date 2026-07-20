#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>

/* LLM input variant 3: zeros-and-ones */

struct Point
{
    float x;
    float y;
};

class VertexAngle
{
public:
    VertexAngle() {}

    // generate a deterministic set of points emphasizing zeros and ones
    void produce_points(int amount)
    {
        for (int idx = 0; idx < amount; ++idx)
        {
            Point pt;
            // coordinates cycle through 0 and 1 to create simple patterns,
            // with occasional repeats that yield zero-length vectors.
            pt.x = static_cast<float>(idx % 2);               // 0,1,0,1,...
            pt.y = static_cast<float>((idx / 2) % 2);        // 0,0,1,1,0,0,1,1,...
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
    // generate 10 deterministic points (will produce 8 angles)
    solver.produce_points(10);
    solver.evaluate();
    return 0;
}
