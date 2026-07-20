#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>

/* LLM input variant 2: small-diverse */

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
    std::srand(static_cast<unsigned>(std::time(0)));

    // ----- deterministic small diverse set of point triples -----
    const int sets = 5;                     // number of triangles
    Pt trio[sets][3] = {
        // Triangle 1: right angle at B (0,0)-(0,1)-(1,0)
        { {0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f} },
        // Triangle 2: acute angle at B
        { {2.0f, 3.0f}, {1.0f, 1.0f}, {4.0f, 2.0f} },
        // Triangle 3: obtuse angle at B
        { {5.0f, 5.0f}, {2.0f, 2.0f}, {8.0f, 1.0f} },
        // Triangle 4: collinear points (angle 0 or 180)
        { {0.0f, 0.0f}, {1.0f, 1.0f}, {2.0f, 2.0f} },
        // Triangle 5: scalene triangle
        { {-3.5f, 2.0f}, {0.0f, -1.0f}, {4.0f, 3.0f} }
    };

    // ----- compute and display each angle -----
    int idx = 0;
    while (idx < sets) {
        float ang = VertexAngle::at(trio[idx][0], trio[idx][1], trio[idx][2]);
        std::cout << "Triangle " << idx + 1 << " – angle at vertex B: "
                  << ang << " degrees\n";
        ++idx;
    }

    return 0;
}
