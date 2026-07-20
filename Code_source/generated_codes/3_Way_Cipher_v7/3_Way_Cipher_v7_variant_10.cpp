#include <iostream>
#include <iomanip>

/* LLM input variant 10: large-safe-stress */

int main() {
    // keys for the 3‑Way Cipher
    int keyX = 0x5A;   // XOR key
    int keyY = 7;      // addition key
    int keyZ = 3;      // rotation bits (left)

    // number of test vectors
    const int tc = 10;
    const int MAX_LEN = 1024; // safe upper bound for each test string

    // stack‑allocated test inputs (max MAX_LEN chars + terminator)
    char src[tc][MAX_LEN + 1] = {
        "", // empty string
        "A", // single character
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.",
        "0123456789""0123456789""0123456789""0123456789""0123456789"
        "0123456789""0123456789""0123456789""0123456789""0123456789", // 100 digits
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()_+-=[]{}|;:',.<>/?`~",
        "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
        "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~", // 200 tildes
        "EdgeCaseTestInputWithMixedChars123!@#",
        "The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog. ",
        "1234567890""1234567890""1234567890""1234567890""1234567890"
        "1234567890""1234567890""1234567890""1234567890""1234567890", // 100 digits
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
        "AAAAAAAAAA" // 200 'A's total
    };

    // compute lengths without library calls
    int lenArr[tc];
    for (int t = 0; t < tc; ++t) {
        int l = 0;
        while (src[t][l] != '\0') ++l;
        lenArr[t] = l;
    }

    // process each test vector
    for (int t = 0; t < tc; ++t) {
        int n = lenArr[t];
        char outArr[MAX_LEN + 1];
        char revArr[MAX_LEN + 1];

        // encryption: XOR → add → rotate left
        for (int idx = 0; idx < n; ++idx) {
            int ch = src[t][idx];
            ch = ch ^ keyX;
            ch = (ch + keyY) & 0xFF;
            ch = ((ch << keyZ) | (ch >> (8 - keyZ))) & 0xFF;
            outArr[idx] = static_cast<char>(ch);
        }

        // decryption: rotate right → subtract → XOR
        for (int idx = 0; idx < n; ++idx) {
            int ch = outArr[idx];
            ch = ((ch >> keyZ) | (ch << (8 - keyZ))) & 0xFF;
            ch = (ch - keyY) & 0xFF;
            ch = ch ^ keyX;
            revArr[idx] = static_cast<char>(ch);
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
