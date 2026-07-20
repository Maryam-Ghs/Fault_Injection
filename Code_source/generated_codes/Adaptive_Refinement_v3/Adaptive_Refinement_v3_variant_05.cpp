/* LLM input variant 5: duplicate-heavy */
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
    float pi = 3.1415926f;
    float tolerance = 1e-5f; // tighter tolerance to force more refinement

    std::vector<float> lowEdges;
    std::vector<float> highEdges;
    std::vector<float> estimates;

    // Duplicate-heavy initial intervals
    lowEdges.push_back(0.0f);
    highEdges.push_back(pi / 2.0f);
    estimates.push_back(simpson(0.0f, pi / 2.0f));

    lowEdges.push_back(0.0f);               // duplicate of first interval start
    highEdges.push_back(pi / 2.0f);
    estimates.push_back(simpson(0.0f, pi / 2.0f));

    lowEdges.push_back(pi / 2.0f);
    highEdges.push_back(pi);
    estimates.push_back(simpson(pi / 2.0f, pi));

    lowEdges.push_back(pi / 2.0f);           // duplicate of third interval start
    highEdges.push_back(pi);
    estimates.push_back(simpson(pi / 2.0f, pi));

    lowEdges.push_back(pi);
    highEdges.push_back(pi);                // zero-length interval (duplicate edges)
    estimates.push_back(simpson(pi, pi));

    lowEdges.push_back(pi);
    highEdges.push_back(pi);
    estimates.push_back(simpson(pi, pi));

    adaptive(lowEdges, highEdges, estimates, tolerance);

    float total = 0.0f;
    for (int i = 0; i < (int)estimates.size(); ++i)
        total += estimates[i];

    std::cout << "Refined integral ≈ " << total << "\n";
    std::cout << "Segments used   = " << (int)estimates.size() << "\n";
    return 0;
}
