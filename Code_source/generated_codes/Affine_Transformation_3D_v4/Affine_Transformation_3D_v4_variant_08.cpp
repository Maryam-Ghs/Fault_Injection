#include <iostream>
#include <vector>

/* LLM input variant 8: sparse-skewed */

int main() {
    // Version #4 – 3D Affine Transformation demo with edge‑case heavy inputs.
    // All data is generated inside the program, no external input.

    // -------------------------------------------------------------------------
    // Prepare a collection of test matrices (4x4) and corresponding points.
    // Each matrix represents an affine transformation in homogeneous coordinates.
    // Sparse and heavily skewed data is used to stress the computation.
    // -------------------------------------------------------------------------

    std::vector<std::vector<std::vector<float>>> transSets;
    std::vector<std::vector<float>> pointSets;

    // 1) Identity matrix (still sparse, only diagonal)
    {
        std::vector<std::vector<float>> m(4, std::vector<float>(4, 0.0f));
        for (int i = 0; i < 4; ++i) m[i][i] = 1.0f;
        transSets.push_back(m);
        pointSets.push_back({ 3.5f, 0.0f, 0.0f, 1.0f });
    }

    // 2) Sparse scaling: only X axis scaled, Y and Z collapsed to zero
    {
        std::vector<std::vector<float>> m(4, std::vector<float>(4, 0.0f));
        m[0][0] = 4.0f;   // scale X by 4
        // Y and Z rows remain zero (collapse)
        m[3][3] = 1.0f;   // homogeneous component
        transSets.push_back(m);
        pointSets.push_back({ 1.0f, 2.0f, 3.0f, 1.0f });
    }

    // 3) Sparse large translation: only X translated far, others untouched
    {
        std::vector<std::vector<float>> m(4, std::vector<float>(4, 0.0f));
        for (int i = 0; i < 4; ++i) m[i][i] = 1.0f;
        m[0][3] = 9e7f;   // huge X translation
        // Y, Z translations left as zero
        transSets.push_back(m);
        pointSets.push_back({ -0.5f, 0.0f, 0.0f, 1.0f });
    }

    // 4) Sparse shear: a single off‑diagonal element creates shear on X from Z
    {
        std::vector<std::vector<float>> m(4, std::vector<float>(4, 0.0f));
        m[0][0] = 1.0f;
        m[1][1] = 1.0f;
        m[2][2] = 1.0f;
        m[0][2] = 0.75f;   // shear X by Z
        m[3][3] = 1.0f;
        transSets.push_back(m);
        pointSets.push_back({ 0.0f, 0.0f, 8.0f, 1.0f });
    }

    // 5) Extremely sparse combination: single non‑zero entry mixes Z into Y,
    //    plus a tiny translation on Z.
    {
        std::vector<std::vector<float>> m(4, std::vector<float>(4, 0.0f));
        m[1][2] = -0.4f;   // mix Z into Y
        m[2][3] = 0.001f; // tiny Z translation
        m[3][3] = 1.0f;
        transSets.push_back(m);
        pointSets.push_back({ 0.0f, 5.0f, 10.0f, 1.0f });
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
