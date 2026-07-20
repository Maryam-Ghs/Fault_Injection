#include <iostream>
#include <vector>
/* LLM input variant 4: signed-extremes */

int main() {
    // -----------------------------------------------------------------
    // 1. Deterministic key (64 bits) and frame number (22 bits) with mixed signed values
    // -----------------------------------------------------------------
    std::vector<int> keyBits(64);
    std::vector<int> frmBits(22);
    for (int i = 0; i < 64; ++i) {
        if (i % 3 == 0) keyBits[i] = -1;          // negative extreme
        else if (i % 3 == 1) keyBits[i] = 0;      // zero
        else keyBits[i] = 1;                      // positive
    }
    for (int i = 0; i < 22; ++i) {
        if (i % 3 == 0) frmBits[i] = -1;
        else if (i % 3 == 1) frmBits[i] = 0;
        else frmBits[i] = 1;
    }

    // -----------------------------------------------------------------
    // 2. Initialise the four LFSRs (A5/2 uses 19, 22, 23 and 17 bits)
    // -----------------------------------------------------------------
    std::vector<int> regA(19, 0), regB(22, 0), regC(23, 0), regD(17, 0);

    // Helper lambda to perform a single shift of a register
    auto shiftReg = [](std::vector<int> &reg, const std::vector<int> &tap) {
        int fb = 0;
        for (size_t i = 0; i < tap.size(); ++i) fb ^= reg[tap[i]];
        for (int j = static_cast<int>(reg.size()) - 1; j > 0; --j) reg[j] = reg[j - 1];
        reg[0] = fb;
    };

    // -----------------------------------------------------------------
    // 3. Load the 64‑bit key (XOR into the LSB, then clock all registers)
    // -----------------------------------------------------------------
    for (int i = 0; i < 64; ++i) {
        regA[0] ^= keyBits[i];
        regB[0] ^= keyBits[i];
        regC[0] ^= keyBits[i];
        regD[0] ^= keyBits[i];

        shiftReg(regA, {13, 16, 17, 18});
        shiftReg(regB, {16, 19, 20, 21});
        shiftReg(regC, {7, 20, 21, 22});
        shiftReg(regD, {13, 16});
    }

    // -----------------------------------------------------------------
    // 4. Load the 22‑bit frame number (same method as the key)
    // -----------------------------------------------------------------
    for (int i = 0; i < 22; ++i) {
        regA[0] ^= frmBits[i];
        regB[0] ^= frmBits[i];
        regC[0] ^= frmBits[i];
        regD[0] ^= frmBits[i];

        shiftReg(regA, {13, 16, 17, 18});
        shiftReg(regB, {16, 19, 20, 21});
        shiftReg(regC, {7, 20, 21, 22});
        shiftReg(regD, {13, 16});
    }

    // -----------------------------------------------------------------
    // 5. Warm‑up phase: 100 irregular clocks (majority rule)
    // -----------------------------------------------------------------
    int warm = 0;
    while (warm < 100) {
        int maj = (regA[8] + regB[10] + regC[10] >= 2) ? 1 : 0;

        if (regA[8] == maj) shiftReg(regA, {13, 16, 17, 18});
        if (regB[10] == maj) shiftReg(regB, {16, 19, 20, 21});
        if (regC[10] == maj) shiftReg(regC, {7, 20, 21, 22});

        // R4 is clocked every round
        shiftReg(regD, {13, 16});
        ++warm;
    }

    // -----------------------------------------------------------------
    // 6. Generate 114 keystream bits (standard GSM burst length)
    // -----------------------------------------------------------------
    std::vector<int> ksBits;
    int produced = 0;
    while (produced < 114) {
        int maj = (regA[8] + regB[10] + regC[10] >= 2) ? 1 : 0;

        if (regA[8] == maj) shiftReg(regA, {13, 16, 17, 18});
        if (regB[10] == maj) shiftReg(regB, {16, 19, 20, 21});
        if (regC[10] == maj) shiftReg(regC, {7, 20, 21, 22});

        shiftReg(regD, {13, 16});

        // output bit is XOR of the most‑significant bits of each register
        int outBit = regA[18] ^ regB[21] ^ regC[22] ^ regD[16];
        ksBits.push_back(outBit);
        ++produced;
    }

    // -----------------------------------------------------------------
    // 7. Print key, frame and resulting keystream
    // -----------------------------------------------------------------
    std::cout << "Key      : ";
    for (int b : keyBits) std::cout << b;
    std::cout << "\nFrame    : ";
    for (int b : frmBits) std::cout << b;
    std::cout << "\nKeystream: ";
    for (int b : ksBits) std::cout << b;
    std::cout << "\n";

    return 0;
}
