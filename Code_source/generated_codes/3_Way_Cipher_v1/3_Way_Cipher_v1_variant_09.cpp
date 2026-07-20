#include <iostream>

/* LLM input variant 9: medium-deterministic-random */

int main() {
    // --- generate medium‑sized deterministic pseudo‑random data on the heap ---
    const int len = 16;
    int* plain = new int[len];
    int* keyA  = new int[len];
    int* keyB  = new int[len];
    int* keyC  = new int[len];

    // deterministic pseudo‑random generator (linear congruential)
    unsigned int seedPlain = 0x1F3A7C9D;
    unsigned int seedA     = 0x4B2E1D3F;
    unsigned int seedB     = 0x8C9D4E2A;
    unsigned int seedC     = 0x5A6B7C8D;

    for (int i = 0; i < len; ++i) {
        // simple LCG: x = (a * x + c) mod 2^32, then take lower 16 bits
        seedPlain = seedPlain * 1664525u + 1013904223u;
        seedA     = seedA     * 22695477u + 1u;
        seedB     = seedB     * 1103515245u + 12345u;
        seedC     = seedC     * 1566083941u + 1u;

        plain[i] = static_cast<int>((seedPlain >> 16) & 0x7FFF); // 0..32767
        keyA[i]  = static_cast<int>((seedA >> 16) & 0x7FFF);
        keyB[i]  = static_cast<int>((seedB >> 16) & 0x7FFF);
        keyC[i]  = static_cast<int>((seedC >> 16) & 0x7FFF);
    }

    // --- encryption (3‑Way Cipher) ---
    int* cipher = new int[len];
    for (int i = 0; i < len; ++i) {
        int step1 = plain[i] + keyA[i];          // add first key
        int step2 = step1 ^ keyB[i];            // XOR with second key
        int step3 = step2 - keyC[i];            // subtract third key
        cipher[i] = step3;                      // store cipher text
    }

    // --- decryption (reverse operations) ---
    int* recovered = new int[len];
    for (int i = 0; i < len; ++i) {
        int rev1 = cipher[i] + keyC[i];         // undo subtraction
        int rev2 = rev1 ^ keyB[i];              // undo XOR
        int rev3 = rev2 - keyA[i];              // undo addition
        recovered[i] = rev3;                    // store recovered plain
    }

    // --- output results ---
    std::cout << "Plain : ";
    for (int i = 0; i < len; ++i) {
        std::cout << plain[i] << ' ';
    }
    std::cout << "\nCipher: ";
    for (int i = 0; i < len; ++i) {
        std::cout << cipher[i] << ' ';
    }
    std::cout << "\nRecover: ";
    for (int i = 0; i < len; ++i) {
        std::cout << recovered[i] << ' ';
    }
    std::cout << std::endl;

    // --- clean up heap memory ---
    delete[] plain;
    delete[] keyA;
    delete[] keyB;
    delete[] keyC;
    delete[] cipher;
    delete[] recovered;

    return 0;
}
