/* LLM input variant 10: large-safe-stress */
#include <iostream>

int main() {
    // ---------- generate edge‑case heavy input ----------
    int testCnt = 1000;
    int* vCount = new int[testCnt];
    // Generate a safe, varied large dataset (values 1..1000 cyclic)
    for (int i = 0; i < testCnt; ++i) {
        vCount[i] = (i % 1000) + 1;
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

    // Output a subset to keep runtime reasonable
    for (int i = 0; i < 20; ++i) {
        std::cout << "Polygon " << i << ": " 
                  << vCount[i] << " vertices, "
                  << "guards needed = " << guardNeed[i] << '\n';
    }
    std::cout << " ... (output truncated) ...\n";

    delete[] vCount;
    delete[] guardNeed;
    return 0;
}
