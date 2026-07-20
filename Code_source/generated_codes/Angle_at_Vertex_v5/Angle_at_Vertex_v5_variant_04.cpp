#include <iostream>
#include <cmath>

/* LLM input variant 4: signed-extremes */

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
    const int sets = 7;                     // number of triangles
    Pt trio[sets][3];                       // stack array: each row holds A, B, C

    // ----- deterministic test data with mixed negative, zero, and positive values -----
    trio[0][0] = { -50.0f, -50.0f };  // A
    trio[0][1] = {  0.0f,   0.0f };   // B
    trio[0][2] = {  50.0f,  50.0f }; // C

    trio[1][0] = { -100.0f,  0.0f };
    trio[1][1] = {   0.0f,  0.0f };
    trio[1][2] = {  100.0f,  0.0f };

    trio[2][0] = {  0.0f, 10.0f };
    trio[2][1] = {  0.0f,  0.0f };
    trio[2][2] = { 10.0f,  0.0f };

    trio[3][0] = { -30.0f,  0.0f };
    trio[3][1] = {  0.0f,  0.0f };
    trio[3][2] = {  0.0f, 30.0f };

    trio[4][0] = { -20.0f, -10.0f };
    trio[4][1] = {  5.0f,   5.0f };
    trio[4][2] = {  20.0f, -10.0f };

    trio[5][0] = { 1000.0f, -1000.0f };
    trio[5][1] = { -500.0f,   0.0f };
    trio[5][2] = {   0.0f,  500.0f };

    trio[6][0] = {   0.0f, -20.0f };
    trio[6][1] = {  10.0f,  0.0f };
    trio[6][2] = {   0.0f,  20.0f };

    // ----- compute and display each angle -----
    for (int idx = 0; idx < sets; ++idx) {
        float ang = VertexAngle::at(trio[idx][0], trio[idx][1], trio[idx][2]);
        std::cout << "Triangle " << idx + 1 << " – angle at vertex B: "
                  << ang << " degrees\n";
    }

    return 0;
}
