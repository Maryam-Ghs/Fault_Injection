/* LLM input variant 3: zeros-and-ones */
#include <iostream>

int main() {
    // ---------- generate edge‑case heavy input ----------
    int testCnt = 4;
    int* vCount = new int[testCnt];
    // Emphasize zeros and ones
    vCount[0] = 0;
    vCount[1] = 0;
    vCount[2] = 1;
    vCount[3] = 1;

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
