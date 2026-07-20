#include <iostream>
#include <vector>
#include <array>

/* LLM input variant 4: signed-extremes */

class Affine3D
{
public:
    // 4x4 matrix stored in row‑major order (16 floats)
    std::vector<float> mat;   // size = 16

    // Build a scaling + translation matrix
    Affine3D(float sx, float sy, float sz,
             float tx, float ty, float tz)
    {
        mat.assign(16, 0.0f);

        // row 0
        mat[0] = sx;   // m00
        mat[1] = 0.0f; // m01
        mat[2] = 0.0f; // m02
        mat[3] = tx;   // m03

        // row 1
        mat[4] = 0.0f; // m10
        mat[5] = sy;   // m11
        mat[6] = 0.0f; // m12
        mat[7] = ty;   // m13

        // row 2
        mat[8]  = 0.0f; // m20
        mat[9]  = 0.0f; // m21
        mat[10] = sz;   // m22
        mat[11] = tz;   // m23

        // row 3
        mat[12] = 0.0f; // m30
        mat[13] = 0.0f; // m31
        mat[14] = 0.0f; // m32
        mat[15] = 1.0f; // m33
    }

    // Apply the affine transform to a list of 3‑D points.
    // Manual loop unrolling is used for the matrix‑vector multiplication.
    void apply(const std::vector<std::array<float,3>>& src,
               std::vector<std::array<float,3>>& dst) const
    {
        // Ensure destination has the right size
        dst.clear();
        dst.reserve(src.size());

        // Unrolled processing of each point
        for (size_t i = 0; i < src.size(); ++i)
        {
            float px = src[i][0];
            float py = src[i][1];
            float pz = src[i][2];

            // Compute X' = m00*px + m01*py + m02*pz + m03*1
            float xp = mat[0]*px + mat[1]*py + mat[2]*pz + mat[3];

            // Compute Y' = m10*px + m11*py + m12*pz + m13*1
            float yp = mat[4]*px + mat[5]*py + mat[6]*pz + mat[7];

            // Compute Z' = m20*px + m21*py + m22*pz + m23*1
            float zp = mat[8]*px + mat[9]*py + mat[10]*pz + mat[11];

            // Store the result
            std::array<float,3> out = { xp, yp, zp };
            dst.push_back(out);
        }
    }
};

int main()
{
    // ---------- generate a tiny data set ----------
    std::vector<std::array<float,3>> points;
    points.push_back({-5.0f, 0.0f, 7.0f});
    points.push_back({0.0f, -3.0f, 0.0f});
    points.push_back({4.5f, -2.5f, 3.0f});

    // ---------- build an affine transform ----------
    // scale by (-2, 1, 0.5) and translate by (0, -1, 5)
    Affine3D xform(-2.0f, 1.0f, 0.5f,
                   0.0f, -1.0f, 5.0f);

    // ---------- apply transformation ----------
    std::vector<std::array<float,3>> transformed;
    xform.apply(points, transformed);

    // ---------- print results ----------
    std::cout << "Original points:\n";
    for (size_t i = 0; i < points.size(); ++i)
    {
        std::cout << "  p" << i << " = ("
                  << points[i][0] << ", "
                  << points[i][1] << ", "
                  << points[i][2] << ")\n";
    }

    std::cout << "\nTransformed points:\n";
    for (size_t i = 0; i < transformed.size(); ++i)
    {
        std::cout << "  p" << i << "' = ("
                  << transformed[i][0] << ", "
                  << transformed[i][1] << ", "
                  << transformed[i][2] << ")\n";
    }

    return 0;
}
