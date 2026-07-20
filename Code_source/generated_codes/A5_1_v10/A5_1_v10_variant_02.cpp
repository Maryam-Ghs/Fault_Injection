/* LLM input variant 2: small-diverse */
// A5/1 stream cipher – version #10
// Everything is inside main, uses only int and float types.

#include <iostream>
#include <cstdlib>
#include <ctime>

#define LEN1 19
#define LEN2 22
#define LEN3 23
#define KEYLEN 64
#define STREAMLEN 64

int main() {
    // Deterministic 64‑bit key (hard‑coded diverse pattern)
    int keybits[KEYLEN] = {
        // alternating groups to provide variety
        0,1,1,0,1,0,0,1,  1,0,0,1,0,1,1,0,  0,0,1,1,  // 20 bits
        1,0,1,0,1,1,0,0,  0,1,0,1,1,0,0,1,  1,0,1,0,  // 20 bits (total 40)
        0,1,0,0,1,1,1,0,  1,0,0,0,1,0,1,1,  0,1,1,0,  // 20 bits (total 60)
        1,0,1,1,0,0,1,0   // remaining 4 bits to reach 64
    };

    // ----- three LFSR registers on the stack -----
    int regA[LEN1];
    int regB[LEN2];
    int regC[LEN3];

    // ----- load registers from the key (simple split) -----
    for (int i = 0; i < LEN1; ++i) regA[i] = keybits[i];
    for (int i = 0; i < LEN2; ++i) regB[i] = keybits[LEN1 + i];
    for (int i = 0; i < LEN3; ++i) regC[i] = keybits[LEN1 + LEN2 + i];

    // ----- helper lambdas for XOR (expanded steps) -----
    auto xor2 = [](int x, int y) -> int { return (x + y) % 2; };
    auto xor3 = [&](int x, int y, int z) -> int {
        int t1 = xor2(x, y);
        return xor2(t1, z);
    };
    auto xor4 = [&](int a, int b, int c, int d) -> int {
        int t1 = xor2(a, b);
        int t2 = xor2(c, d);
        return xor2(t1, t2);
    };

    // ----- generate keystream -----
    int streambits[STREAMLEN];
    for (int outIdx = 0; outIdx < STREAMLEN; ++outIdx) {
        // 1. determine clock bits
        int clkA = regA[8];   // 9th bit (zero‑based)
        int clkB = regB[10];  // 11th bit
        int clkC = regC[10];  // 11th bit

        // 2. majority of the three clock bits
        int sumClk = clkA + clkB + clkC;
        int maj;
        if (sumClk >= 2) maj = 1; else maj = 0;

        // 3. shift registers whose clock bit equals majority
        // ----- Register A -----
        if (clkA == maj) {
            // taps: bits 13,16,17,18
            int tap13 = regA[13];
            int tap16 = regA[16];
            int tap17 = regA[17];
            int tap18 = regA[18];
            int newBitA = xor4(tap13, tap16, tap17, tap18);
            // shift right
            for (int i = LEN1 - 1; i > 0; --i) regA[i] = regA[i - 1];
            regA[0] = newBitA;
        }

        // ----- Register B -----
        if (clkB == maj) {
            // taps: bits 20,21
            int tap20 = regB[20];
            int tap21 = regB[21];
            int newBitB = xor2(tap20, tap21);
            for (int i = LEN2 - 1; i > 0; --i) regB[i] = regB[i - 1];
            regB[0] = newBitB;
        }

        // ----- Register C -----
        if (clkC == maj) {
            // taps: bits 7,20,21,22
            int tap7  = regC[7];
            int tap20 = regC[20];
            int tap21 = regC[21];
            int tap22 = regC[22];
            int newBitC = xor4(tap7, tap20, tap21, tap22);
            for (int i = LEN3 - 1; i > 0; --i) regC[i] = regC[i - 1];
            regC[0] = newBitC;
        }

        // 4. output bit = XOR of the three last bits
        int outA = regA[LEN1 - 1];
        int outB = regB[LEN2 - 1];
        int outC = regC[LEN3 - 1];
        streambits[outIdx] = xor3(outA, outB, outC);
    }

    // ----- print key and keystream as binary strings -----
    std::cout << "Key (64 bits)   : ";
    for (int i = 0; i < KEYLEN; ++i) std::cout << keybits[i];
    std::cout << "\nKeystream (64 bits): ";
    for (int i = 0; i < STREAMLEN; ++i) std::cout << streambits[i];
    std::cout << std::endl;

    return 0;
}
