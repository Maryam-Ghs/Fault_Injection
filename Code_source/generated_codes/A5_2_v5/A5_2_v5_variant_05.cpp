#include <iostream>
#include <vector>

/* LLM input variant 5: duplicate-heavy */

int main() {
    /* -------------------------------------------------------------
       A5/2 stream‑cipher (educational, simplified) implementation.
       All data are stored as vectors of int (bits 0/1).
       No external input – key and frame are predefined small arrays.
       ------------------------------------------------------------- */

    /* ----- 1. Predefined key (64 bits) and frame (22 bits) ----- */
    std::vector<int> keyBits = {
        0,0,0,0, 0,0,0,0,
        1,1,1,1, 1,1,1,1,
        0,0,0,0, 0,0,0,0,
        1,1,1,1, 1,1,1,1,
        0,0,0,0, 0,0,0,0,
        1,1,1,1, 1,1,1,1,
        0,0,0,0, 0,0,0,0,
        1,1,1,1, 1,1,1,1
    };          // 64‑bit key

    std::vector<int> frameBits = {
        0,0,0,0,0,0,0,0,0,0,0,
        1,1,1,1,1,1,1,1,1,1,1
    };          // 22‑bit frame number

    /* ----- 2. Registers R1 (19), R2 (22), R3 (23) ----- */
    std::vector<int> regA(19, 0);   // R1
    std::vector<int> regB(22, 0);   // R2
    std::vector<int> regC(23, 0);   // R3

    /* ----- 3. Helper: majority of three bits ----- */
    auto maj = [&](int x, int y, int z) -> int {
        int sum = x + y + z;                 // reordered arithmetic
        return (sum - (sum / 2) * 2);        // returns 1 if sum is odd (i.e., >=2)
    };

    /* ----- 4. Clock a single register with external mixing bit ----- */
    auto clockReg = [&](std::vector<int>& reg, const std::vector<int>& taps, int mix) {
        int fb = mix;                         // start with mixing bit
        for (size_t i = 0; i < taps.size(); ++i) {
            fb = fb ^ reg[taps[i]];           // XOR all tap positions
        }
        // shift right
        for (int i = (int)reg.size() - 1; i > 0; --i) {
            reg[i] = reg[i - 1];
        }
        reg[0] = fb;                          // new bit enters at position 0
    };

    /* ----- 5. Load the key into the registers ----- */
    int keyIdx = 0;
    while (keyIdx < (int)keyBits.size()) {
        int mixBit = keyBits[keyIdx];
        // clock all three registers (no majority control during loading)
        clockReg(regA, {13, 16, 17, 18}, mixBit);
        clockReg(regB, {7, 19, 20, 21}, mixBit);
        clockReg(regC, {7, 20, 21, 22}, mixBit);
        ++keyIdx;
    }

    /* ----- 6. Load the frame number ----- */
    int frameIdx = 0;
    while (frameIdx < (int)frameBits.size()) {
        int mixBit = frameBits[frameIdx];
        clockReg(regA, {13, 16, 17, 18}, mixBit);
        clockReg(regB, {7, 19, 20, 21}, mixBit);
        clockReg(regC, {7, 20, 21, 22}, mixBit);
        ++frameIdx;
    }

    /* ----- 7. Warm‑up phase (100 cycles) ----- */
    int warm = 0;
    while (warm < 100) {
        // control bits used for majority decision
        int ctrlA = regA[8];
        int ctrlB = regB[10];
        int ctrlC = regC[10];
        int decision = maj(ctrlA, ctrlB, ctrlC);

        if (ctrlA == decision) clockReg(regA, {13, 16, 17, 18}, 0);
        if (ctrlB == decision) clockReg(regB, {7, 19, 20, 21}, 0);
        if (ctrlC == decision) clockReg(regC, {7, 20, 21, 22}, 0);

        ++warm;
    }

    /* ----- 8. Generate keystream (e.g., 20 bits) ----- */
    std::vector<int> keyStream;
    int produced = 0;
    while (produced < 20) {
        // control bits for this cycle
        int ctrlA = regA[8];
        int ctrlB = regB[10];
        int ctrlC = regC[10];
        int decision = maj(ctrlA, ctrlB, ctrlC);

        if (ctrlA == decision) clockReg(regA, {13, 16, 17, 18}, 0);
        if (ctrlB == decision) clockReg(regB, {7, 19, 20, 21}, 0);
        if (ctrlC == decision) clockReg(regC, {7, 20, 21, 22}, 0);

        // output bit is XOR of the three last bits of each register
        int outBit = regA[18] ^ regB[21] ^ regC[22];
        keyStream.push_back(outBit);
        ++produced;
    }

    /* ----- 9. Print the resulting keystream ----- */
    std::cout << "Keystream (20 bits): ";
    for (size_t i = 0; i < keyStream.size(); ++i) {
        std::cout << keyStream[i];
    }
    std::cout << std::endl;

    return 0;
}
