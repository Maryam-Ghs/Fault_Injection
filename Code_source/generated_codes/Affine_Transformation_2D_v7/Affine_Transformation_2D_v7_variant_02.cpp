#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

/* LLM input variant 2: small-diverse */

int main() {
    // version #7
    // generate a small random set of 2‑D points with diverse values
    int pointCount = 15;
    std::vector<std::pair<float, float>> srcPts;
    srcPts.reserve(pointCount);
    std::srand(67890);
    for (int i = 0; i < pointCount; ++i) {
        float rx = static_cast<float>(std::rand() % 401 - 200); // -200 .. 200
        float ry = static_cast<float>(std::rand() % 301 - 150); // -150 .. 150
        srcPts.emplace_back(rx, ry);
    }

    // random affine coefficients (scale/rotate + shear + translation)
    float a = 1.1f;   // m00
    float b = -0.2f;  // m01
    float c = 0.5f;   // m10
    float d = 0.9f;   // m11
    float tx = -12.5f; // translation x
    float ty = 8.3f;   // translation y

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
    int sample = 5;
    for (int i = 0; i < sample; ++i) {
        std::cout << "P" << i << ": (" << dstPts[i].first << ", " << dstPts[i].second << ")\n";
    }

    return 0;
}
