/* LLM input variant 9: medium-deterministic-random */
#include <iostream>

int main() {
    // ---------- generate edge‑case heavy input ----------
    int testCnt = 12;
    int* vCount = new int[testCnt];
    // Deterministic pseudo‑random looking values
    int preset[12] = {13, 27, 4, 19, 31, 8, 22, 5, 16, 9, 33, 2};
    for (int i = 0; i < testCnt; ++i) {
        vCount[i] = preset[i];
    }

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
