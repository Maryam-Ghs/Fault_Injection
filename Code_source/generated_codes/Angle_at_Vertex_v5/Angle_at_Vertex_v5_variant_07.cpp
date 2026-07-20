#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>

/* LLM input variant 7: reverse-adversarial */

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

    // ----- generate a deterministic reverse‑ordered set of point triples -----
    const int sets = 7;                     // number of triangles
    Pt trio[sets][3];                       // stack array: each row holds A, B, C

    // Fill the array in reverse order with colinear points causing a 180° angle at B
    for (int i = sets - 1; i >= 0; --i) {
        // B at origin
        trio[i][1].x = 0.0f;
        trio[i][1].y = 0.0f;
        // A to the right, magnitude decreasing with i
        trio[i][0].x = static_cast<float>(100000 - i * 1000);
        trio[i][0].y = 0.0f;
        // C to the left, magnitude decreasing with i
        trio[i][2].x = static_cast<float>(-100000 + i * 1000);
        trio[i][2].y = 0.0f;
    }

    // ----- compute and display each angle in reverse order -----
    for (int idx = sets - 1; idx >= 0; --idx) {
        float ang = VertexAngle::at(trio[idx][0], trio[idx][1], trio[idx][2]);
        std::cout << "Triangle " << idx + 1 << " – angle at vertex B: "
                  << ang << " degrees\n";
    }

    return 0;
}
