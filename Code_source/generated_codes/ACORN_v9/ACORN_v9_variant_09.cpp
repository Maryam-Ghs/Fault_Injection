#include <iostream>

/* LLM input variant 9: medium-deterministic-random */

int main() {
    int modPow = 1 << 30;                 // 2^30
    int modMask = modPow - 1;             // modulus mask (2^30‑1)
    int sz = 2;                           // order of the generator
    int buf[2];                           // state buffer

    // Deterministic pseudo‑random seed pairs
    const int seedPairs[][2] = {
        {123456789 & ((1 << 30) - 1), 987654321 & ((1 << 30) - 1)},
        {0x1ABCDEF & ((1 << 30) - 1), 0x0FEDCBA & ((1 << 30) - 1)},
        {0x3F3F3F3F & ((1 << 30) - 1), 0xC0C0C0C0 & ((1 << 30) - 1)},
        {0x55555555 & ((1 << 30) - 1), 0xAAAAAAAA & ((1 << 30) - 1)},
        {0x0, (1 << 29)}                  // mix of zero and a large value
    };
    const int numPairs = sizeof(seedPairs) / sizeof(seedPairs[0]);

    for (int p = 0; p < numPairs; ++p) {
        buf[0] = seedPairs[p][0];
        buf[1] = seedPairs[p][1];
        for (int cnt = 0; cnt < 10; ++cnt) {
            int nxt = (buf[0] + buf[1]) & modMask;   // fused sum & modulo
            std::cout << nxt << ' ';
            buf[0] = buf[1];
            buf[1] = nxt;
        }
        std::cout << '\n';
    }

    return 0;
}
