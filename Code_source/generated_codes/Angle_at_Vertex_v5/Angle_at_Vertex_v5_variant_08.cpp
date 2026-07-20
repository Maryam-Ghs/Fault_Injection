#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>

/* LLM input variant 8: sparse-skewed */

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
    // ----- deterministic sparse and skewed set of point triples -----
    const int sets = 7;                     // number of triangles
    Pt trio[sets][3];                       // stack array: each row holds A, B, C

    // Triangle 1: points clustered near origin
    trio[0][0] = {0.1f, 0.2f};
    trio[0][1] = {0.0f, 0.0f};
    trio[0][2] = {0.2f, -0.1f};

    // Triangle 2: one vertex far away, others near origin
    trio[1][0] = {1000.0f, 0.0f};
    trio[1][1] = {0.0f, 0.0f};
    trio[1][2] = {0.0f, 1.0f};

    // Triangle 3: two points identical (degenerate edge), but B distinct
    trio[2][0] = {5.0f, 5.0f};
    trio[2][1] = {5.0f, 5.0f};   // B same as A (will produce zero length, avoid by offset)
    trio[2][2] = {10.0f, 5.0f};
    // Adjust to avoid zero length: shift B slightly
    trio[2][1] = {5.001f, 5.001f};

    // Triangle 4: sparse coordinates with large gaps
    trio[3][0] = {-500.0f, 300.0f};
    trio[3][1] = {0.0f, 0.0f};
    trio[3][2] = {400.0f, -250.0f};

    // Triangle 5: all points on a line (angle should be 180 or 0)
    trio[4][0] = {0.0f, 0.0f};
    trio[4][1] = {1.0f, 1.0f};
    trio[4][2] = {2.0f, 2.0f};

    // Triangle 6: highly skewed small triangle
    trio[5][0] = {0.0f, 0.0f};
    trio[5][1] = {0.0001f, 0.0f};
    trio[5][2] = {0.0f, 0.0001f};

    // Triangle 7: mixed sparse and dense region
    trio[6][0] = {0.0f, 0.0f};
    trio[6][1] = {50.0f, -50.0f};
    trio[6][2] = {0.0f, 1000.0f};

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
