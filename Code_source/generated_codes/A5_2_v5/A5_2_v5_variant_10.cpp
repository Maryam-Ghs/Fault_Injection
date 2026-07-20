#include <iostream>
#include <vector>

/* LLM input variant 10: large-safe-stress */

int main() {
    /* -------------------------------------------------------------
       A5/2 stream‑cipher (educational, simplified) implementation.
       All data are stored as vectors of int (bits 0/1).
       No external input – key and frame are predefined large arrays.
       ------------------------------------------------------------- */

    /* ----- 1. Generated key (1024 bits) and frame (256 bits) ----- */
    std::vector<int> keyBits;
    keyBits.reserve(1024);
    for (int i = 0; i < 1024; ++i) {
        // Simple deterministic pattern: XOR of bit i%2 and (i/3)%2
        keyBits.push_back( (i % 2) ^ ((i / 3) % 2) );
    }

    std::vector<int> frameBits;
    frameBits.reserve(256);
    for (int i = 0; i < 256; ++i) {
        // Another deterministic pattern: parity of i
        frameBits.push_back( (i % 2) );
    }

    /* ----- 2. Registers R1 (19), R2 (22), R3 (23) ----- */
    std::vector<int> regA(19, 0);   // R1
    std::vector<int> regB(22, 0);   // R2
    std::vector<int> regC(23, 0);   // R3

    /* ----- 3. Helper: majority of three bits ----- */
    auto maj = [&](int x, int y, int z) -> int {
        int sum = x + y + z;
        return (sum - (sum / 2) * 2);        // returns 1 if sum is odd (i.e., >=2)
    };

    /* ----- 4. Clock a single register with external mixing bit ----- */
    auto clockReg = [&](std::vector<int>& reg, const std::vector<int>& taps, int mix) {
        int fb = mix;
        for (size_t i = 0; i < taps.size(); ++i) {
            fb = fb ^ reg[taps[i]];
        }
        for (int i = static_cast<int>(reg.size()) - 1; i > 0; --i) {
            reg[i] = reg[i - 1];
        }
        reg[0] = fb;
    };

    /* ----- 5. Load the key into the registers ----- */
    for (size_t keyIdx = 0; keyIdx < keyBits.size(); ++keyIdx) {
        int mixBit = keyBits[keyIdx];
        clockReg(regA, {13, 16, 17, 18}, mixBit);
        clockReg(regB, {7, 19, 20, 21}, mixBit);
        clockReg(regC, {7, 20, 21, 22}, mixBit);
    }

    /* ----- 6. Load the frame number ----- */
    for (size_t frameIdx = 0; frameIdx < frameBits.size(); ++frameIdx) {
        int mixBit = frameBits[frameIdx];
        clockReg(regA, {13, 16, 17, 18}, mixBit);
        clockReg(regB, {7, 19, 20, 21}, mixBit);
        clockReg(regC, {7, 20, 21, 22}, mixBit);
    }

    /* ----- 7. Warm‑up phase (1000 cycles) ----- */
    for (int warm = 0; warm < 1000; ++warm) {
        int ctrlA = regA[8];
        int ctrlB = regB[10];
        int ctrlC = regC[10];
        int decision = maj(ctrlA, ctrlB, ctrlC);

        if (ctrlA == decision) clockReg(regA, {13, 16, 17, 18}, 0);
        if (ctrlB == decision) clockReg(regB, {7, 19, 20, 21}, 0);
        if (ctrlC == decision) clockReg(regC, {7, 20, 21, 22}, 0);
    }

    /* ----- 8. Generate keystream (200 bits) ----- */
    std::vector<int> keyStream;
    keyStream.reserve(200);
    for (int produced = 0; produced < 200; ++produced) {
        int ctrlA = regA[8];
        int ctrlB = regB[10];
        int ctrlC = regC[10];
        int decision = maj(ctrlA, ctrlB, ctrlC);

        if (ctrlA == decision) clockReg(regA, {13, 16, 17, 18}, 0);
        if (ctrlB == decision) clockReg(regB, {7, 19, 20, 21}, 0);
        if (ctrlC == decision) clockReg(regC, {7, 20, 21, 22}, 0);

        int outBit = regA[18] ^ regB[21] ^ regC[22];
        keyStream.push_back(outBit);
    }

    /* ----- 9. Print the resulting keystream ----- */
    std::cout << "Keystream (200 bits): ";
    for (size_t i = 0; i < keyStream.size(); ++i) {
        std::cout << keyStream[i];
    }
    std::cout << std::endl;

    return 0;
}
