#include <iostream>
#include <vector>

/* LLM input variant 9: medium-deterministic-random */

int main() {
    // Version #4 – 3D Affine Transformation demo with edge‑case heavy inputs.
    // All data is generated inside the program, no external input.

    // -------------------------------------------------------------------------
    // Prepare a collection of test matrices (4x4) and corresponding points.
    // Each matrix represents an affine transformation in homogeneous coordinates.
    // -------------------------------------------------------------------------

    std::vector<std::vector<std::vector<float>>> transSets;
    std::vector<std::vector<float>> pointSets;

    // 1) Non‑uniform scaling with translation
    {
        std::vector<std::vector<float>> m(4, std::vector<float>(4, 0.0f));
        m[0][0] = 3.0f;   // scale X
        m[1][1] = -2.0f;  // scale Y (negative flips)
        m[2][2] = 0.5f;   // scale Z
        m[3][3] = 1.0f;   // homogeneous
        m[0][3] = 5.0f;   // translate X
        m[1][3] = -3.0f;  // translate Y
        m[2][3] = 2.0f;   // translate Z
        transSets.push_back(m);
        pointSets.push_back({ 1.2f, -4.5f, 3.3f, 1.0f });
    }

    // 2) Rotation about Z by 45° with translation
    {
        const float c = 0.70710678f; // cos(45°)
        const float s = 0.70710678f; // sin(45°)
        std::vector<std::vector<float>> m(4, std::vector<float>(4, 0.0f));
        m[0][0] = c;   m[0][1] = -s;  m[0][2] = 0.0f; m[0][3] = -2.0f;
        m[1][0] = s;   m[1][1] =  c;  m[1][2] = 0.0f; m[1][3] =  4.0f;
        m[2][2] = 1.0f;
        m[3][3] = 1.0f;
        transSets.push_back(m);
        pointSets.push_back({ -1.0f, 2.0f, 5.0f, 1.0f });
    }

    // 3) Shear transformations (X by Y, Y by Z) with unit scaling
    {
        std::vector<std::vector<float>> m(4, std::vector<float>(4, 0.0f));
        m[0][0] = 1.0f;   m[0][1] = 1.5f;   // shear X based on Y
        m[1][1] = 1.0f;   m[1][2] = -0.8f;  // shear Y based on Z
        m[2][2] = 1.0f;
        m[3][3] = 1.0f;
        transSets.push_back(m);
        pointSets.push_back({ 2.0f, 3.0f, -1.0f, 1.0f });
    }

    // 4) Slight perspective effect (non‑identity last row)
    {
        std::vector<std::vector<float>> m(4, std::vector<float>(4, 0.0f));
        m[0][0] = 1.0f;   m[0][3] = 100.0f;
        m[1][1] = 1.0f;   m[1][3] = -50.0f;
        m[2][2] = 1.0f;   m[2][3] = 25.0f;
        m[3][0] = 0.001f; m[3][1] = 0.002f; m[3][2] = 0.003f; m[3][3] = 1.0f;
        transSets.push_back(m);
        pointSets.push_back({ 10.0f, 20.0f, 30.0f, 1.0f });
    }

    // 5) Mixed large values and negative scaling
    {
        std::vector<std::vector<float>> m(4, std::vector<float>(4, 0.0f));
        m[0][0] = -12.5f;  m[0][1] =  3.4f;   m[0][2] = 0.0f;   m[0][3] = 800.0f;
        m[1][0] =  4.2f;   m[1][1] = -9.1f;   m[1][2] = 2.5f;   m[1][3] = -600.0f;
        m[2][0] =  0.0f;   m[2][1] =  1.7f;   m[2][2] = 5.5f;   m[2][3] = 400.0f;
        m[3][3] = 1.0f;
        transSets.push_back(m);
        pointSets.push_back({ -7.0f, 8.5f, -3.3f, 1.0f });
    }

    // -------------------------------------------------------------------------
    // Process each test case using a loop‑heavy, direct‑computation approach.
    // -------------------------------------------------------------------------
    for (size_t idx = 0; idx < transSets.size(); ++idx) {
        const std::vector<std::vector<float>>& curM = transSets[idx];
        const std::vector<float>& srcP = pointSets[idx];

        // Result vector (4 components)
        std::vector<float> dstP(4, 0.0f);

        // Standard matrix‑vector multiplication:
        // dstP[j] = Σ_i curM[j][i] * srcP[i]
        for (int j = 0; j < 4; ++j) {
            float acc = 0.0f;
            for (int i = 0; i < 4; ++i) {
                acc += curM[j][i] * srcP[i];
            }
            dstP[j] = acc;
        }

        // Output the transformation result.
        std::cout << "Case " << idx + 1 << ":\n";
        std::cout << "  Input point  : ("
                  << srcP[0] << ", " << srcP[1] << ", " << srcP[2] << ")\n";
        std::cout << "  Transformed  : ("
                  << dstP[0] << ", " << dstP[1] << ", " << dstP[2] << ")\n";
        std::cout << "  Homogeneous w: " << dstP[3] << "\n\n";
    }

    return 0;
}
