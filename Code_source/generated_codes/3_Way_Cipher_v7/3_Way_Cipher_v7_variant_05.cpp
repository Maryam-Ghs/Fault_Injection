/* LLM input variant 5: duplicate-heavy */
#include <iostream>
#include <iomanip>

int main() {
    // keys for the 3‑Way Cipher
    int keyX = 0x5A;   // XOR key
    int keyY = 7;      // addition key
    int keyZ = 3;      // rotation bits (left)

    // number of test vectors
    int tc = 5;

    // stack‑allocated test inputs (max 255 chars + terminator)
    char src[5][256] = {
        "",                                 // empty string
        "AAAAAAAAAA",                       // 10 repeated 'A's
        "BBBBBBBBBB",                       // 10 repeated 'B's
        "CCCCCCCCCCCCCCCCCCCC",             // 20 repeated 'C's
        "AAAAAAAAAA"                        // duplicate of test 1
    };

    // compute lengths without library calls
    int lenArr[5];
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
