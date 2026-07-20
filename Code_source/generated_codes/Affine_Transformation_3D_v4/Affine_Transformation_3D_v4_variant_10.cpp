#include <iostream>
#include <vector>

/* LLM input variant 10: large-safe-stress */

int main() {
    // Version #4 – 3D Affine Transformation demo with edge‑case heavy inputs.
    // All data is generated inside the program, no external input.

    // -------------------------------------------------------------------------
    // Generate a larger collection of test matrices (4x4) and corresponding points.
    // Each matrix represents an affine transformation in homogeneous coordinates.
    // -------------------------------------------------------------------------

    std::vector<std::vector<std::vector<float>>> transSets;
    std::vector<std::vector<float>> pointSets;

    const int NUM_CASES = 20;  // relatively large but still safe for memory and runtime

    for (int t = 0; t < NUM_CASES; ++t) {
        // Create a 4x4 matrix initialized to zeros
        std::vector<std::vector<float>> m(4, std::vector<float>(4, 0.0f));

        // Determine a scaling factor that varies with the test index
        float scale = 1.0f + static_cast<float>(t) * 0.05f;
        if (t % 4 == 1) scale = -scale;               // occasional negative scaling
        if (t % 4 == 2) scale = 0.0f;                 // occasional zero scaling (collapse)

        // Apply scaling (or collapse) to the diagonal for X, Y, Z axes
        for (int i = 0; i < 3; ++i) m[i][i] = scale;

        // Homogeneous component
        m[3][3] = 1.0f;

        // Large translation components (grow with t)
        m[0][3] = static_cast<float>(t) * 1e5f;           // translate X
        m[1][3] = -static_cast<float>(t) * 1e5f;          // translate Y
        m[2][3] = (t % 2 == 0) ? 5e4f : -5e4f;            // translate Z alternating sign

        // Add a shear component on every 5th case
        if (t % 5 == 0) {
            m[0][1] = 0.2f;   // shear X based on Y
            m[1][0] = -0.2f;  // shear Y based on X
        }

        transSets.push_back(m);

        // Construct a point that also varies with the test index
        float x = (t + 1) * 3.1415f;   // multiples of pi
        float y = (t + 1) * -2.718f;  // multiples of -e
        float z = (t + 1) * 0.5772f;  // multiples of 1/√3
        pointSets.push_back({ x, y, z, 1.0f });
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
