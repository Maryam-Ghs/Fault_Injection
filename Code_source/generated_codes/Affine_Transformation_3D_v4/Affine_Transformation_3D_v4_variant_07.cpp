#include <iostream>
#include <vector>

/* LLM input variant 7: reverse-adversarial */

int main() {
    // Version #4 – 3D Affine Transformation demo with edge‑case heavy inputs.
    // All data is generated inside the program, no external input.

    // -------------------------------------------------------------------------
    // Prepare a collection of test matrices (4x4) and corresponding points.
    // Each matrix represents an affine transformation in homogeneous coordinates.
    // -------------------------------------------------------------------------

    std::vector<std::vector<std::vector<float>>> transSets;
    std::vector<std::vector<float>> pointSets;

    // 5) Combination: scaling + rotation‑like mixing + translation (now first, with reversed assignments)
    {
        std::vector<std::vector<float>> m(4, std::vector<float>(4, 0.0f));
        // Translation (set first to test ordering)
        m[3][3] = 1.0f;
        m[2][3] = 3.0f;
        m[1][3] = -5.0f;
        m[0][3] = 10.0f;
        // Rotate-ish mixing X↔Y (reverse order)
        m[1][0] = -0.3f;
        m[0][1] = 0.3f;
        // Scale X by 2, Y by 0.5, Z by -1 (reverse loop)
        m[2][2] = -1.0f;
        m[1][1] = 0.5f;
        m[0][0] = 2.0f;
        transSets.push_back(m);
        pointSets.push_back({ -2.0f, 4.0f, 0.5f, 1.0f });
    }

    // 4) Singular matrix (last row zeroed – no perspective division) (reversed rows)
    {
        std::vector<std::vector<float>> m(4, std::vector<float>(4, 0.0f));
        // Fill rows in reverse order
        m[2][2] = 1.0f;
        m[1][1] = 1.0f;
        // Simple shear on X based on Y (reverse column order)
        m[0][1] = 2.0f;
        m[0][0] = 1.0f;
        // last row left as zeros (singular)
        transSets.push_back(m);
        pointSets.push_back({ 1.0f, 1.0f, 1.0f, 1.0f });
    }

    // 3) Large translation (tests overflow‑like behavior with floats) (reverse order of setting)
    {
        std::vector<std::vector<float>> m(4, std::vector<float>(4, 0.0f));
        for (int i = 3; i >= 0; --i) m[i][i] = 1.0f;
        // Set translation components in reverse index order
        m[2][3] = 5e5f;    // translate Z by half a million
        m[1][3] = -1e6f;   // translate Y by minus a million
        m[0][3] = 1e6f;    // translate X by a million
        transSets.push_back(m);
        pointSets.push_back({ -123.45f, 678.90f, -0.12f, 1.0f });
    }

    // 2) Zero scaling (collapses space to a single point) (reverse loop)
    {
        std::vector<std::vector<float>> m(4, std::vector<float>(4, 0.0f));
        for (int i = 2; i >= 0; --i) m[i][i] = 0.0f;   // scale axes to 0
        m[3][3] = 1.0f;                               // homogeneous component
        transSets.push_back(m);
        pointSets.push_back({ 9.0f, -4.0f, 2.0f, 1.0f });
    }

    // 1) Anti‑diagonal identity (reflection) (reverse ordering)
    {
        std::vector<std::vector<float>> m(4, std::vector<float>(4, 0.0f));
        for (int i = 0; i < 4; ++i) m[i][3 - i] = 1.0f; // anti‑diagonal ones
        transSets.push_back(m);
        pointSets.push_back({ 3.5f, -2.1f, 7.0f, 1.0f });
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
