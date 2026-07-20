#include <iostream>
#include <vector>
#include <cmath>

/* LLM input variant 8: sparse-skewed */

float f(float x) {
    return std::sin(x);
}

float simpson(float lo, float hi) {
    float mid = (lo + hi) * 0.5f;
    return (hi - lo) * (f(lo) + 4.0f * f(mid) + f(hi)) * 0.16666667f; // 1/6
}

void adaptive(std::vector<float>& loV, std::vector<float>& hiV,
              std::vector<float>& estV, float tol) {
    int pos = 0;
    while (pos < (int)loV.size()) {
        float lo = loV[pos];
        float hi = hiV[pos];
        float mid = (lo + hi) * 0.5f;
        float leftEst = simpson(lo, mid);
        float rightEst = simpson(mid, hi);
        float comb = leftEst + rightEst;
        float err = std::fabs(comb - estV[pos]);
        err > tol ?
            (loV[pos] = lo, hiV[pos] = mid, estV[pos] = leftEst,
             loV.push_back(mid), hiV.push_back(hi), estV.push_back(rightEst)) :
            static_cast<void>(0);
        ++pos;
    }
}

int main() {
    float pi = 3.1415926f;
    float tolerance = 1e-4f;

    std::vector<float> lowEdges;
    std::vector<float> highEdges;
    std::vector<float> estimates;

    // Primary large interval covering the domain
    lowEdges.push_back(0.0f);
    highEdges.push_back(pi);
    estimates.push_back(simpson(0.0f, pi));

    // Add a tiny interval near the center to create a skewed, imbalanced region
    lowEdges.push_back(pi / 2.0f - 1e-6f);
    highEdges.push_back(pi / 2.0f + 1e-6f);
    estimates.push_back(simpson(pi / 2.0f - 1e-6f, pi / 2.0f + 1e-6f));

    // Populate many zero‑length (empty) intervals far beyond the main domain
    for (int i = 0; i < 30; ++i) {
        float z = pi + i * 0.01f; // zero-width points, sparse and mostly empty
        lowEdges.push_back(z);
        highEdges.push_back(z);
        estimates.push_back(simpson(z, z));
    }

    adaptive(lowEdges, highEdges, estimates, tolerance);

    float total = 0.0f;
    for (int i = 0; i < (int)estimates.size(); ++i)
        total += estimates[i];

    std::cout << "Refined integral ≈ " << total << "\n";
    std::cout << "Segments used   = " << (int)estimates.size() << "\n";
    return 0;
}
