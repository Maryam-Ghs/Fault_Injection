/* LLM input variant 5: duplicate-heavy */
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

int main() {
    // version #7
    // generate a deterministic set of 2‑D points with many duplicates
    int pointCount = 10000;
    std::vector<std::pair<float, float>> srcPts;
    srcPts.reserve(pointCount);

    // a small pool of points that will be repeated
    const std::vector<std::pair<float, float>> basePoints = {
        {0.0f, 0.0f},
        {10.0f, -10.0f},
        {20.0f, 20.0f},
        {-15.0f, 5.0f},
        {0.0f, 0.0f}  // duplicate of the first point
    };
    const int baseSize = static_cast<int>(basePoints.size());

    for (int i = 0; i < pointCount; ++i) {
        srcPts.emplace_back(basePoints[i % baseSize]);
    }

    // random affine coefficients (scale/rotate + shear + translation)
    float a = 0.8f;   // m00
    float b = 0.3f;   // m01
    float c = -0.4f;  // m10
    float d = 1.2f;   // m11
    float tx = 15.0f; // translation x
    float ty = -7.0f; // translation y

    // destination container
    std::vector<std::pair<float, float>> dstPts(pointCount);

    // apply the affine transformation
    // split into temporaries, no branches inside the loop
    for (int i = 0; i < pointCount; ++i) {
        float ox = srcPts[i].first;
        float oy = srcPts[i].second;

        float p1 = a * ox;        // a * x
        float p2 = b * oy;        // b * y
        float p3 = c * ox;        // c * x
        float p4 = d * oy;        // d * y

        float nx = p1 + p2 + tx;  // (a*x + b*y) + tx
        float ny = p3 + p4 + ty;  // (c*x + d*y) + ty

        dstPts[i].first  = nx;
        dstPts[i].second = ny;
    }

    // print a subset of the transformed points
    int sample = 10;
    for (int i = 0; i < sample; ++i) {
        std::cout << "P" << i << ": (" << dstPts[i].first << ", " << dstPts[i].second << ")\n";
    }

    return 0;
}
