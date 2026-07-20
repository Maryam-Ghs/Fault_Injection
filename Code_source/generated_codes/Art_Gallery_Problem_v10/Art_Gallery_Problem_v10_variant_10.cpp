#include <iostream>
#include <cmath>   // only for std::abs, still works with float/int

/* LLM input variant 10: large-safe-stress */

// ------------------------------------------------------------
// Simple point structure (heap‑allocated)
// ------------------------------------------------------------
struct Pt {
    float x;
    float y;
};

// ------------------------------------------------------------
// Build a large regular polygon (1000 vertices) safely
// ------------------------------------------------------------
Pt* buildPoly(int &sz) {
    sz = 1000;
    Pt* ptr = new Pt[sz];
    const float radius = 100.0f;
    const float pi = 3.14159265358979323846f;
    int i = 0;
    while (i < sz) {
        float angle = 2 * pi * i / sz;
        ptr[i].x = radius * std::cos(angle);
        ptr[i].y = radius * std::sin(angle);
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
