#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <array>

/* LLM input variant 10: large-safe-stress */

struct Pt {
    float x;
    float y;
};

class VertexAngle {
public:
    // Compute the angle (in degrees) at the middle point B of triangle ABC
    static float at(const Pt& A, const Pt& B, const Pt& C) {
        // ----- vector components -----
        float vx1 = A.x - B.x;
        float vy1 = A.y - B.y;
        float vx2 = C.x - B.x;
        float vy2 = C.y - B.y;

        // ----- dot product -----
        float dot = vx1 * vx2 + vy1 * vy2;

        // ----- lengths (squared) -----
        float lenSq1 = vx1 * vx1 + vy1 * vy1;
        float lenSq2 = vx2 * vx2 + vy2 * vy2;

        // ----- lengths (actual) -----
        float len1 = std::sqrt(lenSq1);
        float len2 = std::sqrt(lenSq2);

        // ----- cosine of the angle -----
        float cosTheta = dot / (len1 * len2);

        // ----- clamp to avoid domain errors -----
        if (cosTheta > 1.0f) cosTheta = 1.0f;
        if (cosTheta < -1.0f) cosTheta = -1.0f;

        // ----- angle in radians -----
        float rad = std::acos(cosTheta);

        // ----- convert to degrees -----
        float deg = rad * (180.0f / 3.14159265358979323846f);
        return deg;
    }
};

int main() {
    // Fixed seed for deterministic behavior
    std::srand(12345);

    // ----- generate a large safe set of point triples -----
    const int sets = 10000;                     // number of triangles
    std::vector<std::array<Pt, 3>> trio(sets); // heap allocation

    for (int i = 0; i < sets; ++i) {
        for (int j = 0; j < 3; ++j) {
            // random coordinates in range [-1000, 1000]
            trio[i][j].x = static_cast<float>((std::rand() % 2001) - 1000);
            trio[i][j].y = static_cast<float>((std::rand() % 2001) - 1000);
        }
    }

    // ----- compute and display each angle -----
    for (int idx = 0; idx < sets; ++idx) {
        float ang = VertexAngle::at(trio[idx][0], trio[idx][1], trio[idx][2]);
        std::cout << "Triangle " << idx + 1 << " – angle at vertex B: "
                  << ang << " degrees\n";
    }

    return 0;
}
