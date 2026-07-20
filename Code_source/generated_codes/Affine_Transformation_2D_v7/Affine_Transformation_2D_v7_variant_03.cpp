#include <iostream>
#include <vector>
/* LLM input variant 3: zeros-and-ones */

int main() {
    // version #7
    // generate a deterministic set of 2‑D points emphasizing zeros and ones
    int pointCount = 20;
    std::vector<std::pair<float, float>> srcPts;
    srcPts.reserve(pointCount);
    for (int i = 0; i < pointCount; ++i) {
        float rx = (i % 3 == 0) ? 0.0f : ((i % 3 == 1) ? 1.0f : -1.0f);
        float ry = (i % 2 == 0) ? 0.0f : 1.0f;
        srcPts.emplace_back(rx, ry);
    }

    // identity affine coefficients (scale/rotate + shear + translation)
    float a = 1.0f;   // m00
    float b = 0.0f;   // m01
    float c = 0.0f;   // m10
    float d = 1.0f;   // m11
    float tx = 0.0f; // translation x
    float ty = 0.0f; // translation y

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
    for (int i = 0; i < sample && i < pointCount; ++i) {
        std::cout << "P" << i << ": (" << dstPts[i].first << ", " << dstPts[i].second << ")\n";
    }

    return 0;
}
