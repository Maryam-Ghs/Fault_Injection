/* LLM input variant 2: small-diverse */
#include <iostream>

int main() {
    // ---------- generate edge‑case heavy input ----------
    // Number of test polygons
    int testCnt = 5;
    // Allocate array for vertex counts on the heap
    int* vCount = new int[testCnt];
    // Small, varied ordinary values
    vCount[0] = 1;   // single point
    vCount[1] = 2;   // line segment
    vCount[2] = 4;   // quadrilateral
    vCount[3] = 7;   // heptagon
    vCount[4] = 10;  // decagon

    // Allocate array for results on the heap
    int* guardNeed = new int[testCnt];

    // ---------- compute guards using manual loop unrolling ----------
    int idx = 0;
    while (idx + 1 < testCnt) {
        int n0 = vCount[idx];
        guardNeed[idx] = (n0 - n0 % 3) / 3;
        int n1 = vCount[idx + 1];
        guardNeed[idx + 1] = (n1 - n1 % 3) / 3;
        idx += 2;
    }
    if (idx < testCnt) {
        int nLast = vCount[idx];
        guardNeed[idx] = (nLast - nLast % 3) / 3;
    }

    // ---------- output results ----------
    for (int i = 0; i < testCnt; ++i) {
        std::cout << "Polygon " << i << ": " 
                  << vCount[i] << " vertices, "
                  << "guards needed = " << guardNeed[i] << '\n';
    }

    delete[] vCount;
    delete[] guardNeed;
    return 0;
}
