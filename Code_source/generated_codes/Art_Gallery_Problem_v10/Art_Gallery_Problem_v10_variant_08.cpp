#include <iostream>
#include <cmath>   // only for std::abs, still works with float/int

/* LLM input variant 8: sparse-skewed */

// ------------------------------------------------------------
// Simple point structure (heap‑allocated)
// ------------------------------------------------------------
struct Pt {
    float x;
    float y;
};

// ------------------------------------------------------------
// Build a sparsely distributed, skewed polygon
// ------------------------------------------------------------
Pt* buildPoly(int &sz) {
    sz = 8;
    Pt* ptr = new Pt[sz];
    // Cluster of points near origin
    ptr[0].x = 0.1f;  ptr[0].y = 0.1f;
    ptr[1].x = 0.2f;  ptr[1].y = 0.1f;
    ptr[2].x = 0.15f; ptr[2].y = 0.25f;
    // Sparse far‑away points creating skew
    ptr[3].x = 10.0f; ptr[3].y = 0.0f;
    ptr[4].x = 12.0f; ptr[4].y = 5.0f;
    ptr[5].x = 11.0f; ptr[5].y = 9.0f;
    ptr[6].x = 8.0f;  ptr[6].y = 7.0f;
    ptr[7].x = 9.5f;  ptr[7].y = 2.5f;
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
