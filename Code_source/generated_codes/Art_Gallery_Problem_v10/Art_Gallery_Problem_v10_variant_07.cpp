#include <iostream>
#include <cmath>   // only for std::abs, still works with float/int

/* LLM input variant 7: reverse-adversarial */

// ------------------------------------------------------------
// Simple point structure (heap‑allocated)
// ------------------------------------------------------------
struct Pt {
    float x;
    float y;
};

// ------------------------------------------------------------
// Build a polygon with vertices in reverse (clockwise) order
// ------------------------------------------------------------
Pt* buildPoly(int &sz) {
    sz = 7;
    Pt* ptr = new Pt[sz];
    // Define points in a simple convex shape but store them reversed
    float xs[7] = {0.0f, 6.0f, 7.0f, 4.0f, 2.0f, -1.0f, -2.0f};
    float ys[7] = {0.0f, 0.0f, 3.0f, 5.0f, 5.0f, 2.0f, 1.0f};
    int i = sz - 1;
    while (i >= 0) {
        ptr[sz - 1 - i].x = xs[i];
        ptr[sz - 1 - i].y = ys[i];
        --i;
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
