#include <iostream>
#include <iomanip>

/* LLM input variant 1: minimal-boundary */

int main() {
    // keys for the 3‑Way Cipher
    int keyX = 0x5A;   // XOR key
    int keyY = 7;      // addition key
    int keyZ = 3;      // rotation bits (left)

    // number of test vectors
    int tc = 2;

    // stack‑allocated test inputs (max 255 chars + terminator)
    char src[2][256] = {
        "",   // empty string (boundary)
        "Z"   // single character (minimal nontrivial)
    };

    // compute lengths without library calls
    int lenArr[2];
    for (int t = 0; t < tc; ++t) {
        int l = 0;
        while (src[t][l] != '\0') ++l;
        lenArr[t] = l;
    }

    // process each test vector
    for (int t = 0; t < tc; ++t) {
        int n = lenArr[t];
        char outArr[256];
        char revArr[256];

        // encryption: XOR → add → rotate left
        for (int idx = 0; idx < n; ++idx) {
            int ch = src[t][idx];
            ch = ch ^ keyX;
            ch = (ch + keyY) & 0xFF;
            ch = ((ch << keyZ) | (ch >> (8 - keyZ))) & 0xFF;
            outArr[idx] = (char)ch;
        }

        // decryption: rotate right → subtract → XOR
        for (int idx = 0; idx < n; ++idx) {
            int ch = outArr[idx];
            ch = ((ch >> keyZ) | (ch << (8 - keyZ))) & 0xFF;
            ch = (ch - keyY) & 0xFF;
            ch = ch ^ keyX;
            revArr[idx] = (char)ch;
        }

        // null‑terminate strings
        outArr[n] = '\0';
        revArr[n] = '\0';

        // output results
        std::cout << "Test " << t << ":\n";
        std::cout << "  Input    : \"" << src[t] << "\"\n";
        std::cout << "  Cipher   : ";
        for (int i = 0; i < n; ++i) {
            int val = outArr[i] & 0xFF;
            std::cout << std::hex << std::setw(2) << std::setfill('0') << val << ' ';
        }
        std::cout << std::dec << "\n";
        std::cout << "  Decoded  : \"" << revArr << "\"\n\n";
    }
    return 0;
}
