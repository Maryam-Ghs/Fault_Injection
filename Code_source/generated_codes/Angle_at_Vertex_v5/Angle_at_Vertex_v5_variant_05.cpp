#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>

/* LLM input variant 5: duplicate-heavy */

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

    // ----- generate a medium‑sized deterministic set of point triples with many duplicates -----
    const int sets = 7;                     // number of triangles
    Pt trio[sets][3];                       // stack array: each row holds A, B, C

    // Define a duplicate‑heavy pattern: A and C are identical, B repeats across triangles
    Pt dupA = {10.0f, -10.0f};
    Pt dupB = {0.0f, 0.0f};
    Pt dupC = {10.0f, -10.0f};

    int i = 0;
    while (i < sets) {
        // Assign the same pattern to every triangle
        trio[i][0] = dupA;  // A
        trio[i][1] = dupB;  // B
        trio[i][2] = dupC;  // C
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
