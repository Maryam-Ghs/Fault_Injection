/* LLM input variant 6: ordered-structured */
#include <iostream>

int main() {
    // ---------- generate edge‑case heavy input ----------
    int testCnt = 7;
    int* vCount = new int[testCnt];
    // Sorted ascending, symmetric pattern
    vCount[0] = 3;
    vCount[1] = 6;
    vCount[2] = 9;
    vCount[3] = 12;
    vCount[4] = 12;
    vCount[5] = 9;
    vCount[6] = 6;

    int* guardNeed = new int[testCnt];

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

    for (int i = 0; i < testCnt; ++i) {
        std::cout << "Polygon " << i << ": " 
                  << vCount[i] << " vertices, "
                  << "guards needed = " << guardNeed[i] << '\n';
    }

    delete[] vCount;
    delete[] guardNeed;
    return 0;
}
