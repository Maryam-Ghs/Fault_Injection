#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>

/* LLM input variant 9: medium-deterministic-random */

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
    // ----- generate a medium‑sized deterministic set of point triples -----
    const int sets = 7;                     // number of triangles
    Pt trio[sets][3];                       // stack array: each row holds A, B, C

    int i = 0;
    while (i < sets) {
        int j = 0;
        while (j < 3) {
            // deterministic pseudo‑random coordinates in range [-50, 50]
            int base = (i * 23 + j * 31) % 101;        // yields 0‑100
            trio[i][j].x = static_cast<float>(base - 50);
            int baseY = (i * 37 + j * 43) % 101;
            trio[i][j].y = static_cast<float>(baseY - 50);
            ++j;
        }
        ++i;
    }

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
