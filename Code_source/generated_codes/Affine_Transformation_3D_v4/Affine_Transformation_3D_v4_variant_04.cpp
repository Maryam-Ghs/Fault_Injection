#include <iostream>
#include <vector>

/* LLM input variant 4: signed-extremes */

int main() {
    // Version #4 – 3D Affine Transformation demo with edge‑case heavy inputs.
    // All data is generated inside the program, no external input.

    // -------------------------------------------------------------------------
    // Prepare a collection of test matrices (4x4) and corresponding points.
    // Each matrix represents an affine transformation in homogeneous coordinates.
    // -------------------------------------------------------------------------

    std::vector<std::vector<std::vector<float>>> transSets;
    std::vector<std::vector<float>> pointSets;

    // 1) Identity matrix with extreme signed point values
    {
        std::vector<std::vector<float>> m(4, std::vector<float>(4, 0.0f));
        for (int i = 0; i < 4; ++i) m[i][i] = 1.0f;
        transSets.push_back(m);
        pointSets.push_back({ -3.4e38f, 0.0f, 3.4e38f, 1.0f });
    }

    // 2) Zero scaling (collapses space to a single point) with mixed signs
    {
        std::vector<std::vector<float>> m(4, std::vector<float>(4, 0.0f));
        for (int i = 0; i < 3; ++i) m[i][i] = 0.0f;   // scale axes to 0
        m[3][3] = 1.0f;                               // homogeneous component
        transSets.push_back(m);
        pointSets.push_back({ -12345.0f, 0.0f, 67890.0f, 1.0f });
    }

    // 3) Large translation with signed‑extreme offsets
    {
        std::vector<std::vector<float>> m(4, std::vector<float>(4, 0.0f));
        for (int i = 0; i < 4; ++i) m[i][i] = 1.0f;
        m[0][3] = 2.0e38f;    // translate X by a huge positive
        m[1][3] = -2.0e38f;   // translate Y by a huge negative
        m[2][3] = 1.0e38f;    // translate Z by a large positive
        transSets.push_back(m);
        pointSets.push_back({ -1.5e19f, 2.5e19f, -3.0e19f, 1.0f });
    }

    // 4) Singular matrix with extreme shear values
    {
        std::vector<std::vector<float>> m(4, std::vector<float>(4, 0.0f));
        // Shear on X based on Y with a very large factor
        m[0][0] = 1.0f;
        m[0][1] = 3.4e38f;   // extreme positive shear
        m[1][1] = -3.4e38f;  // extreme negative scaling on Y
        m[2][2] = 1.0f;
        // last row left as zeros (singular)
        transSets.push_back(m);
        pointSets.push_back({ 1.0f, 1.0f, 1.0f, 1.0f });
    }

    // 5) Combination: scaling + rotation‑like mixing + translation with signed extremes
    {
        std::vector<std::vector<float>> m(4, std::vector<float>(4, 0.0f));
        // Scale X by -2, Y by 0 (zero scaling), Z by 1e20
        m[0][0] = -2.0f;
        m[1][1] = 0.0f;
        m[2][2] = 1.0e20f;
        // Rotate‑ish mixing X↔Y with moderate coefficients
        m[0][1] = 0.5f;
        m[1][0] = -0.5f;
        // Translation with mixed signed extremes
        m[0][3] = -9.9e37f;
        m[1][3] = 9.9e37f;
        m[2][3] = -1.0e20f;
        m[3][3] = 1.0f;
        transSets.push_back(m);
        pointSets.push_back({ -2.0f, 4.0f, 0.5f, 1.0f });
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
