#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

/* LLM input variant 9: medium-deterministic-random */

int main() {
    // version #7
    // generate a medium deterministic pseudo‑random set of 2‑D points
    int pointCount = 8000;
    std::vector<std::pair<float, float>> srcPts;
    srcPts.reserve(pointCount);

    // simple deterministic LCG for reproducible pseudo‑random numbers
    unsigned int seed = 0xDEADBEEFu;
    for (int i = 0; i < pointCount; ++i) {
        seed = seed * 1664525u + 1013904223u;
        float rx = static_cast<float>((seed % 200u) - 100u);
        seed = seed * 1664525u + 1013904223u;
        float ry = static_cast<float>((seed % 200u) - 100u);
        srcPts.emplace_back(rx, ry);
    }

    // deterministic affine coefficients (scale/rotate + shear + translation)
    float a = 0.75f;   // m00
    float b = -0.45f;  // m01
    float c = 0.50f;   // m10
    float d = 0.90f;   // m11
    float tx = -12.3f; // translation x
    float ty =  8.7f;  // translation y

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
