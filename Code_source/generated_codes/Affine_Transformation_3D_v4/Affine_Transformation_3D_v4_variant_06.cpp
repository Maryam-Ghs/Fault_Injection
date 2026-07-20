/* LLM input variant 6: ordered-structured */
#include <iostream>
#include <vector>

int main() {
    // Version #4 – 3D Affine Transformation demo with ordered‑structured inputs.
    // All data is generated inside the program, no external input.

    // -------------------------------------------------------------------------
    // Prepare a collection of test matrices (4x4) and corresponding points.
    // Each matrix is crafted to exhibit sorted, symmetric, or otherwise highly
    // structured patterns.
    // -------------------------------------------------------------------------

    std::vector<std::vector<std::vector<float>>> transSets;
    std::vector<std::vector<float>> pointSets;

    // 1) Diagonal scaling matrix with increasing diagonal entries.
    {
        std::vector<std::vector<float>> m(4, std::vector<float>(4, 0.0f));
        m[0][0] = 1.0f;   // X scale
        m[1][1] = 2.0f;   // Y scale
        m[2][2] = 3.0f;   // Z scale
        m[3][3] = 1.0f;   // homogeneous component
        transSets.push_back(m);
        pointSets.push_back({ 1.0f, 2.0f, 3.0f, 1.0f }); // sorted ascending
    }

    // 2) Upper‑triangular matrix with strictly increasing row values.
    {
        std::vector<std::vector<float>> m(4, std::vector<float>(4, 0.0f));
        float val = 1.0f;
        for (int i = 0; i < 4; ++i) {
            for (int j = i; j < 4; ++j) {
                m[i][j] = val;
                val += 1.0f; // monotonic increase
            }
        }
        transSets.push_back(m);
        pointSets.push_back({ -5.0f, -3.0f, -1.0f, 1.0f }); // descending negatives
    }

    // 3) Symmetric positive‑definite matrix (also serves as a simple shear+scale).
    {
        std::vector<std::vector<float>> m = {
            { 4.0f, 1.0f, 0.0f, 0.0f },
            { 1.0f, 3.0f, 1.0f, 0.0f },
            { 0.0f, 1.0f, 2.0f, 0.0f },
            { 0.0f, 0.0f, 0.0f, 1.0f }
        };
        transSets.push_back(m);
        pointSets.push_back({ 0.0f, 1.0f, 2.0f, 1.0f }); // monotonic
    }

    // 4) Near‑identity matrix with small ordered perturbations.
    {
        std::vector<std::vector<float>> m(4, std::vector<float>(4, 0.0f));
        for (int i = 0; i < 4; ++i) m[i][i] = 1.0f;
        // Add tiny ordered offsets
        m[0][1] = 0.01f;
        m[1][2] = 0.02f;
        m[2][3] = 0.03f;
        m[3][0] = 0.04f;
        transSets.push_back(m);
        pointSets.push_back({ 10.0f, 20.0f, 30.0f, 1.0f }); // ascending multiples of 10
    }

    // 5) Rotation about Z axis by 45 degrees combined with translation.
    {
        const float c = 0.70710678f; // cos(45°)
        const float s = 0.70710678f; // sin(45°)
        std::vector<std::vector<float>> m = {
            {  c, -s, 0.0f, 5.0f },
            {  s,  c, 0.0f, -5.0f },
            { 0.0f, 0.0f, 1.0f, 2.0f },
            { 0.0f, 0.0f, 0.0f, 1.0f }
        };
        transSets.push_back(m);
        pointSets.push_back({ 1.0f, 1.0f, 0.0f, 1.0f }); // symmetric point
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
