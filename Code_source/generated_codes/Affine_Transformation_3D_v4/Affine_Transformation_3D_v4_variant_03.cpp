#include <iostream>
#include <vector>

/* LLM input variant 3: zeros-and-ones */

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
        pointSets.push_back({ 0.0f, 0.0f, 0.0f, 1.0f });
    }

    // 2) Zero scaling (collapses space to a single point)
    {
        std::vector<std::vector<float>> m(4, std::vector<float>(4, 0.0f));
        // first three diagonal entries remain 0 (scale to 0)
        m[3][3] = 1.0f; // homogeneous component
        transSets.push_back(m);
        pointSets.push_back({ 0.0f, 0.0f, 0.0f, 1.0f });
    }

    // 3) Unit translation (adds 1 to each spatial coordinate)
    {
        std::vector<std::vector<float>> m(4, std::vector<float>(4, 0.0f));
        for (int i = 0; i < 4; ++i) m[i][i] = 1.0f;
        m[0][3] = 1.0f; // translate X by 1
        m[1][3] = 1.0f; // translate Y by 1
        m[2][3] = 1.0f; // translate Z by 1
        transSets.push_back(m);
        pointSets.push_back({ 0.0f, 0.0f, 0.0f, 1.0f });
    }

    // 4) Simple shear with singular last row
    {
        std::vector<std::vector<float>> m(4, std::vector<float>(4, 0.0f));
        m[0][0] = 1.0f; m[0][1] = 1.0f; // shear X by Y
        m[1][1] = 1.0f;
        m[2][2] = 1.0f;
        // last row remains all zeros (singular)
        transSets.push_back(m);
        pointSets.push_back({ 1.0f, 1.0f, 1.0f, 1.0f });
    }

    // 5) Axis swap (X↔Y) with identity elsewhere
    {
        std::vector<std::vector<float>> m(4, std::vector<float>(4, 0.0f));
        m[0][1] = 1.0f; // X gets Y
        m[1][0] = 1.0f; // Y gets X
        m[2][2] = 1.0f;
        m[3][3] = 1.0f;
        transSets.push_back(m);
        pointSets.push_back({ 1.0f, 0.0f, 1.0f, 1.0f });
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
