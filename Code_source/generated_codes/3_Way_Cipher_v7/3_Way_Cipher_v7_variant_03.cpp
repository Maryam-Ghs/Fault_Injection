#include <iostream>
#include <iomanip>

/* LLM input variant 3: zeros-and-ones */

int main() {
    // keys for the 3‑Way Cipher (identity values)
    int keyX = 0x00;   // XOR key
    int keyY = 0;      // addition key
    int keyZ = 0;      // rotation bits (left)

    // number of test vectors
    int tc = 4;

    // stack‑allocated test inputs (max 255 chars + terminator)
    char src[4][256] = {
        "", // empty string
        "0", // single zero character
        "1111111111111111111111111111111111111111111111111111111111111111", // long ones
        "0101010101010101010101010101010101010101010101010101010101010101"  // alternating zeros and ones
    };

    // compute lengths without library calls
    int lenArr[4];
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
