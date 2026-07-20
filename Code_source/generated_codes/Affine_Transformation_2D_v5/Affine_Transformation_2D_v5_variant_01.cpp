/*
 * LLM input variant 1: minimal-boundary
 *
 * Affine Transformation 2D – version #5
 * -------------------------------------------------
 * Implements a 2‑D affine transform using only int and float,
 * heavy loop usage, reordered arithmetic, std::vector storage,
 * and a class‑based design.
 *
 * No external input – all data is generated inside main().
 */

#include <iostream>
#include <vector>
#include <cmath>
#include <climits>

class Affine2D {
public:
    // 3×3 matrix stored row‑major in a flat vector
    std::vector<float> mat;   // size = 9

    Affine2D() {
        mat.resize(9);
        setIdentity();
    }

    void setIdentity() {
        // Fill with identity matrix
        for (int i = 0; i < 9; ++i) mat[i] = 0.0f;
        mat[0] = 1.0f; mat[4] = 1.0f; mat[8] = 1.0f;
    }

    void setScale(float sx, float sy) {
        setIdentity();
        mat[0] = sx;      // a
        mat[4] = sy;      // e
    }

    void setRotate(float deg) {
        setIdentity();
        float rad = deg * (3.14159265358979323846f / 180.0f);
        float c = cosf(rad);
        float s = sinf(rad);
        // reordered multiplication order
        mat[0] = c;       // a
        mat[1] = -s;      // b
        mat[3] = s;       // d
        mat[4] = c;       // e
    }

    void setTranslate(float tx, float ty) {
        setIdentity();
        mat[2] = tx;      // c
        mat[5] = ty;      // f
    }

    // Combine this matrix with another: this = this * other
    void combine(const Affine2D& other) {
        std::vector<float> res(9, 0.0f);
        // Triple nested loops – heavily iterative
        for (int row = 0; row < 3; ++row) {
            for (int col = 0; col < 3; ++col) {
                for (int k = 0; k < 3; ++k) {
                    // reorder: (a*b)+c -> a*b + c
                    res[row * 3 + col] = res[row * 3 + col] + 
                                         (mat[row * 3 + k] * other.mat[k * 3 + col]);
                }
            }
        }
        mat.swap(res);
    }

    // Apply transformation to a point (x,y)
    void apply(float& x, float& y) const {
        // Reordered arithmetic: ((c + a*x) + b*y)
        float nx = ((mat[2] + mat[0] * x) + mat[1] * y);
        float ny = ((mat[5] + mat[3] * x) + mat[4] * y);
        x = nx;
        y = ny;
    }
};

struct Point2D {
    float x;
    float y;
};

int main() {
    // -------------------------------------------------
    // 1. Generate minimal‑size edge‑case points
    // -------------------------------------------------
    std::vector<Point2D> pts;
    pts.push_back({0.0f, 0.0f});                                   // origin
    pts.push_back({1.0f, 1.0f});                                   // simple
    pts.push_back({-1.0f, -1.0f});                                 // negative
    pts.push_back({static_cast<float>(INT_MAX), 0.0f});           // max int as float
    pts.push_back({0.0f, static_cast<float>(INT_MIN)});           // min int as float

    // Minimal bulk loop (single iteration) to keep loop structure
    for (int i = 0; i <= 0; ++i) {
        for (int j = 0; j <= 0; ++j) {
            pts.push_back({static_cast<float>(i), static_cast<float>(j)});
        }
    }

    // -------------------------------------------------
    // 2. Build a composite transformation:
    //    scale → rotate → translate
    // -------------------------------------------------
    Affine2D scaleT;   scaleT.setScale(2.0f, -3.0f);
    Affine2D rotT;     rotT.setRotate(90.0f);   // 90 degrees
    Affine2D transT;   transT.setTranslate(100.0f, -50.0f);

    // Compose: first scale, then rotate, then translate
    Affine2D composite;
    composite.combine(scaleT);
    composite.combine(rotT);
    composite.combine(transT);

    // -------------------------------------------------
    // 3. Apply transformation to all points (loop‑heavy)
    // -------------------------------------------------
    std::vector<Point2D> outPts;
    outPts.reserve(pts.size());

    for (size_t idx = 0; idx < pts.size(); ++idx) {
        float x = pts[idx].x;
        float y = pts[idx].y;
        // Apply composite transform
        composite.apply(x, y);
        outPts.push_back({x, y});
    }

    // -------------------------------------------------
    // 4. Print results (original → transformed)
    // -------------------------------------------------
    for (size_t i = 0; i < pts.size(); ++i) {
        std::cout << "In: (" << pts[i].x << ", " << pts[i].y << ")  ";
        std::cout << "Out: (" << outPts[i].x << ", " << outPts[i].y << ")\n";
    }

    return 0;
}
