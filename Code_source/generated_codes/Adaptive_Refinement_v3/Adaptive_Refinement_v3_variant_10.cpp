/* LLM input variant 10: large-safe-stress */
#include <iostream>
#include <vector>
#include <cmath>

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
    const float pi = 3.1415926f;
    const float domain_end = 10.0f * pi;  // larger interval for stress testing
    const float tolerance = 1e-5f;        // tighter tolerance to increase subdivisions

    std::vector<float> lowEdges;
    std::vector<float> highEdges;
    std::vector<float> estimates;

    // Initialize with multiple sub-intervals covering the whole domain
    const int initial_segments = 10;
    for (int i = 0; i < initial_segments; ++i) {
        float lo = i * pi;
        float hi = (i + 1) * pi;
        lowEdges.push_back(lo);
        highEdges.push_back(hi);
        estimates.push_back(simpson(lo, hi));
    }

    adaptive(lowEdges, highEdges, estimates, tolerance);

    float total = 0.0f;
    for (size_t i = 0; i < estimates.size(); ++i)
        total += estimates[i];

    std::cout << "Refined integral ≈ " << total << "\n";
    std::cout << "Segments used   = " << (int)estimates.size() << "\n";
    return 0;
}
