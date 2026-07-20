#include <iostream>
#include <vector>

/* LLM input variant 2: small-diverse */

int main() {
    // Version #4 – 3D Affine Transformation demo with edge‑case heavy inputs.
    // All data is generated inside the program, no external input.

    // -------------------------------------------------------------------------
    // Prepare a collection of test matrices (4x4) and corresponding points.
    // Each matrix represents an affine transformation in homogeneous coordinates.
    // -------------------------------------------------------------------------

    std::vector<std::vector<std::vector<float>>> transSets;
    std::vector<std::vector<float>> pointSets;

    // 1) Identity matrix (should leave point unchanged)
    {
        std::vector<std::vector<float>> m(4, std::vector<float>(4, 0.0f));
        for (int i = 0; i < 4; ++i) m[i][i] = 1.0f;
        transSets.push_back(m);
        pointSets.push_back({ 1.2f, -3.4f, 5.6f, 1.0f });
    }

    // 2) Uniform scaling by 2 (expands space)
    {
        std::vector<std::vector<float>> m(4, std::vector<float>(4, 0.0f));
        for (int i = 0; i < 3; ++i) m[i][i] = 2.0f;   // scale X,Y,Z by 2
        m[3][3] = 1.0f;
        transSets.push_back(m);
        pointSets.push_back({ -1.0f, 2.0f, -3.0f, 1.0f });
    }

    // 3) Moderate translation (tests basic offset handling)
    {
        std::vector<std::vector<float>> m(4, std::vector<float>(4, 0.0f));
        for (int i = 0; i < 4; ++i) m[i][i] = 1.0f;
        m[0][3] = 100.0f;   // translate X by 100
        m[1][3] = -50.0f;   // translate Y by -50
        m[2][3] = 25.0f;    // translate Z by 25
        transSets.push_back(m);
        pointSets.push_back({ 4.5f, 0.0f, -7.5f, 1.0f });
    }

    // 4) Simple shear (X depends on Y, Y depends on Z)
    {
        std::vector<std::vector<float>> m(4, std::vector<float>(4, 0.0f));
        m[0][0] = 1.0f; m[0][1] = 1.5f;   // X' = X + 1.5*Y
        m[1][1] = 1.0f; m[1][2] = 0.5f;   // Y' = Y + 0.5*Z
        m[2][2] = 1.0f;
        m[3][3] = 1.0f;
        transSets.push_back(m);
        pointSets.push_back({ 2.0f, 3.0f, 4.0f, 1.0f });
    }

    // 5) Combination: non‑uniform scaling, mixing, and translation
    {
        std::vector<std::vector<float>> m(4, std::vector<float>(4, 0.0f));
        // Scaling
        m[0][0] = 0.5f;   // X scaled by 0.5
        m[1][1] = 3.0f;   // Y scaled by 3
        m[2][2] = -2.0f;  // Z scaled by -2
        // Mixing (approximate rotation in XY plane)
        m[0][1] = 0.7f;
        m[1][0] = -0.7f;
        // Translation
        m[0][3] = 5.0f;
        m[1][3] = -5.0f;
        m[2][3] = 10.0f;
        m[3][3] = 1.0f;
        transSets.push_back(m);
        pointSets.push_back({ -3.0f, 1.0f, 2.0f, 1.0f });
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
