#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

/* LLM input variant 4: signed-extremes */

int main() {
    // version #7
    // deterministic set of 2‑D points covering negative, zero, and positive extremes
    int pointCount = 9;
    std::vector<std::pair<float, float>> srcPts;
    srcPts.reserve(pointCount);
    srcPts.emplace_back(-100.0f, -100.0f);
    srcPts.emplace_back(-100.0f,   0.0f);
    srcPts.emplace_back(-100.0f,  100.0f);
    srcPts.emplace_back(   0.0f, -100.0f);
    srcPts.emplace_back(   0.0f,   0.0f);
    srcPts.emplace_back(   0.0f,  100.0f);
    srcPts.emplace_back( 100.0f, -100.0f);
    srcPts.emplace_back( 100.0f,   0.0f);
    srcPts.emplace_back( 100.0f,  100.0f);

    // mixed-sign affine coefficients (including zero)
    float a = -1.0f;   // m00
    float b =  0.0f;   // m01
    float c =  0.5f;   // m10
    float d =  0.0f;   // m11
    float tx =  0.0f;  // translation x
    float ty = -50.0f; // translation y

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

    // print all transformed points
    int sample = pointCount;
    for (int i = 0; i < sample; ++i) {
        std::cout << "P" << i << ": (" << dstPts[i].first << ", " << dstPts[i].second << ")\n";
    }

    return 0;
}
