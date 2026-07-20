#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

/* LLM input variant 10: large-safe-stress */

int main() {
    // version #7
    // generate a large random set of 2‑D points
    int pointCount = 500000; // increased size for stress testing
    std::vector<std::pair<float, float>> srcPts;
    srcPts.reserve(pointCount);
    std::srand(987654321);
    for (int i = 0; i < pointCount; ++i) {
        float rx = static_cast<float>(std::rand() % 2000 - 1000); // wider range
        float ry = static_cast<float>(std::rand() % 2000 - 1000);
        srcPts.emplace_back(rx, ry);
    }

    // random affine coefficients (scale/rotate + shear + translation)
    float a = 1.1f;   // m00
    float b = -0.2f;   // m01
    float c = 0.5f;  // m10
    float d = 0.9f;   // m11
    float tx = -12.5f; // translation x
    float ty = 23.3f; // translation y

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
