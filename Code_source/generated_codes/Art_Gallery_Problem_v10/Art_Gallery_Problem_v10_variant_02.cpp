#include <iostream>
#include <cmath>   // only for std::abs, still works with float/int

/* LLM input variant 2: small-diverse */

// ------------------------------------------------------------
// Simple point structure (heap‑allocated)
// ------------------------------------------------------------
struct Pt {
    float x;
    float y;
};

// ------------------------------------------------------------
// Build a small diverse polygon (hexagon) on the heap
// ------------------------------------------------------------
Pt* buildPoly(int &sz) {
    sz = 6;
    Pt* ptr = new Pt[sz];

    int i = 0;
    while (i < sz) {
        if (i == 0) { ptr[i].x = 0.0f;   ptr[i].y = 0.0f; }
        else if (i == 1) { ptr[i].x = 3.0f;   ptr[i].y = 0.0f; }
        else if (i == 2) { ptr[i].x = 5.5f;   ptr[i].y = 2.0f; }
        else if (i == 3) { ptr[i].x = 4.0f;   ptr[i].y = 4.5f; }
        else if (i == 4) { ptr[i].x = 1.5f;   ptr[i].y = 4.0f; }
        else { ptr[i].x = -0.5f;  ptr[i].y = 2.5f; }
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
