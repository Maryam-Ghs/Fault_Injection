#include <iostream>
#include <cmath>   // only for std::abs, still works with float/int

/* LLM input variant 9: medium-deterministic-random */

// ------------------------------------------------------------
// Simple point structure (heap‑allocated)
// ------------------------------------------------------------
struct Pt {
    float x;
    float y;
};

// ------------------------------------------------------------
// Build a deterministic pseudo‑random looking polygon (15 vertices)
// ------------------------------------------------------------
Pt* buildPoly(int &sz) {
    sz = 15;
    Pt* ptr = new Pt[sz];
    // Pre‑computed pseudo‑random values (modulo some range)
    float xs[15] = {0.0f, 2.3f, 4.7f, 3.1f, 5.5f, 7.8f, 6.0f, 8.2f, 9.4f, 7.1f, 5.9f, 4.2f, 2.8f, 1.1f, 0.5f};
    float ys[15] = {0.0f, 1.9f, 3.4f, 5.0f, 4.2f, 2.7f, 1.5f, 3.3f, 5.5f, 6.8f, 5.2f, 3.9f, 2.1f, 0.8f, 0.0f};
    int i = 0;
    while (i < sz) {
        ptr[i].x = xs[i];
        ptr[i].y = ys[i];
        ++i;
    }
    return ptr;
}

// ------------------------------------------------------------
// Compute polygon area using the shoelace formula (float math)
// ------------------------------------------------------------
float polyArea(const Pt* p, int n) {
    float sum = 0.0f;
    int k = 0;
    while (k < n) {
        int nxt = (k + 1) % n;
        float term1 = p[k].x * p[nxt].y;
        float term2 = p[nxt].x * p[k].y;
        sum = sum + term1 - term2;
        ++k;
    }
    float absSum = std::abs(sum);
    float half = 0.5f;
    float area = absSum * half;
    return area;
}

// ------------------------------------------------------------
// Determine the minimum number of guards (Art Gallery theorem)
// ------------------------------------------------------------
int minGuards(int vCount) {
    int three = 3;
    int div = vCount / three;
    int mult = div * three;
    int leftover = vCount - mult;
    int guards = div;
    return guards;
}

// ------------------------------------------------------------
// Pretty‑print the outcome
// ------------------------------------------------------------
void showResult(int guardCnt, float areaVal) {
    std::cout << "Polygon vertices : " << guardCnt * 3 << " (approx.)\n";
    std::cout << "Computed area    : " << areaVal << "\n";
    std::cout << "Needed guards    : " << guardCnt << "\n";
}

// ------------------------------------------------------------
// Main driver (no cin, all data generated inside)
// ------------------------------------------------------------
int main() {
    int sz = 0;
    Pt* poly = buildPoly(sz);
    float a = polyArea(poly, sz);
    int g = minGuards(sz);
    showResult(g, a);
    delete [] poly;
    return 0;
}
