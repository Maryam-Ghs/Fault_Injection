/* LLM input variant 7: reverse-adversarial */
#include <iostream>

int main() {
    // ---------- generate edge‑case heavy input ----------
    int testCnt = 6;
    int* vCount = new int[testCnt];
    // Reverse-ordered values, worst-case for some algorithms
    vCount[0] = 1000;
    vCount[1] = 999;
    vCount[2] = 500;
    vCount[3] = 499;
    vCount[4] = 10;
    vCount[5] = 3;

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
