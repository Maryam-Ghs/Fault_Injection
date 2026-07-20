#include <iostream>
#include <vector>

/* LLM input variant 5: duplicate-heavy */

int main() {
    // Version #4 – 3D Affine Transformation demo with edge‑case heavy inputs.
    // All data is generated inside the program, no external input.

    // -------------------------------------------------------------------------
    // Prepare a collection of test matrices (4x4) and corresponding points.
    // Each matrix represents an affine transformation in homogeneous coordinates.
    // -------------------------------------------------------------------------

    std::vector<std::vector<std::vector<float>>> transSets;
    std::vector<std::vector<float>> pointSets;

    // 1) Uniform matrix (all entries the same) – extreme duplication
    {
        std::vector<std::vector<float>> m(4, std::vector<float>(4, 1.0f));
        // Keep homogeneous component consistent
        m[3][3] = 1.0f;
        transSets.push_back(m);
        pointSets.push_back({ 5.0f, 5.0f, 5.0f, 1.0f });
    }

    // 2) Zero‑scaling with duplicated zero rows
    {
        std::vector<std::vector<float>> m(4, std::vector<float>(4, 0.0f));
        // First three rows identical zero rows (already zero)
        // Homogeneous row unchanged
        m[3][3] = 1.0f;
        transSets.push_back(m);
        pointSets.push_back({ 7.0f, 7.0f, 7.0f, 1.0f });
    }

    // 3) Large uniform translation (same value on all axes)
    {
        std::vector<std::vector<float>> m(4, std::vector<float>(4, 0.0f));
        for (int i = 0; i < 4; ++i) m[i][i] = 1.0f;
        // Same large translation for X, Y, Z
        m[0][3] = 1e6f;
        m[1][3] = 1e6f;
        m[2][3] = 1e6f;
        transSets.push_back(m);
        pointSets.push_back({ -100.0f, -100.0f, -100.0f, 1.0f });
    }

    // 4) Singular matrix with duplicated rows
    {
        std::vector<std::vector<float>> m(4, std::vector<float>(4, 0.0f));
        // Rows 0 and 1 are identical
        m[0][0] = 1.0f; m[0][1] = 1.0f;
        m[1][0] = 1.0f; m[1][1] = 1.0f;
        // Row 2 distinct but simple
        m[2][2] = 1.0f;
        // Last row left zero (singular)
        transSets.push_back(m);
        pointSets.push_back({ 2.0f, 2.0f, 2.0f, 1.0f });
    }

    // 5) Combination with many repeated constants
    {
        std::vector<std::vector<float>> m(4, std::vector<float>(4, 0.0f));
        // Uniform scaling factor duplicated
        m[0][0] = 2.0f;
        m[1][1] = 2.0f;
        m[2][2] = 2.0f;
        // Mixing values repeated
        m[0][1] = 0.5f;
        m[1][0] = 0.5f;
        m[0][2] = 0.5f;
        m[2][0] = 0.5f;
        // Uniform translation duplicated
        m[0][3] = 3.0f;
        m[1][3] = 3.0f;
        m[2][3] = 3.0f;
        m[3][3] = 1.0f;
        transSets.push_back(m);
        pointSets.push_back({ 3.0f, 3.0f, 3.0f, 1.0f });
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
