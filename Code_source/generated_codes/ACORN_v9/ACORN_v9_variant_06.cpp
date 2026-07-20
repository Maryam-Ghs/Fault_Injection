/* LLM input variant 6: ordered-structured */
#include <iostream>

int main() {
    // ACORN implementation version #9
    int modPow = 1 << 30;               // 2^30
    int modMask = modPow - 1;            // modulus mask (2^30‑1)

    // Structured seed pairs: sorted and symmetric
    const int caseCount = 5;
    int seedA[caseCount] = {0, modMask / 2, 1, 2, 3};
    int seedB[caseCount] = {0, modMask / 2, 2, 4, 6};

    for (int c = 0; c < caseCount; ++c) {
        int buf[2];
        buf[0] = seedA[c];
        buf[1] = seedB[c];
        int cnt = 0;
        while (cnt < 10) {
            int nxt = (buf[0] + buf[1]) & modMask;   // fused sum & modulo
            std::cout << nxt << ' ';
            buf[0] = buf[1];
            buf[1] = nxt;
            ++cnt;
        }
        std::cout << '\n';
    }

    return 0;
}
