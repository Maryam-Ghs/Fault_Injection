/* LLM input variant 1: minimal-boundary */
#include <iostream>

int main() {
    // ---------- generate edge‑case heavy input ----------
    // Number of test polygons
    int testCnt = 1;
    // Allocate array for vertex counts on the heap
    int* vCount = new int[testCnt];
    // Fill with the smallest valid nontrivial boundary value
    vCount[0] = 0;    // no polygon (boundary)

    // Allocate array for results on the heap
    int* guardNeed = new int[testCnt];

    // ---------- compute guards using manual loop unrolling ----------
    int idx = 0;
    // Process pairs of entries per iteration
    while (idx + 1 < testCnt) {
        int n0 = vCount[idx];
        guardNeed[idx] = (n0 - n0 % 3) / 3;
        int n1 = vCount[idx + 1];
        guardNeed[idx + 1] = (n1 - n1 % 3) / 3;
        idx += 2;
    }
    // Handle possible leftover element when testCnt is odd
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

    // ---------- clean up ----------
    delete[] vCount;
    delete[] guardNeed;
    return 0;
}
