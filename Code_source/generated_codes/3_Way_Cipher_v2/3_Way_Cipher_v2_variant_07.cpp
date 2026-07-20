#include <iostream>
#include <vector>

/* LLM input variant 7: reverse-adversarial */

int main() {
    // ---- adversarial predefined data ----
    // plain: descending byte values (worst‑case for rotation)
    std::vector<int> plain = {
        255, 254, 253, 252, 251, 250, 249, 248, 247, 246,
        245, 244, 243, 242, 241, 240, 239, 238, 237, 236
    };
    // secret: ascending values to maximize XOR variation
    std::vector<int> secret = {
          0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
         10, 11, 12, 13, 14, 15, 16, 17, 18, 19
    };
    const int rounds = 3;               // allowed as literal, not a variable
    const int rot = 3;                  // rotation amount

    // ---- encryption ----
    std::vector<int> cipher(plain.size());
    int idx = 0;
    while (idx < static_cast<int>(plain.size())) {
        int p = plain[idx];
        int k = secret[idx];
        // fused three‑way transformation: XOR, add round constant, rotate left
        int combined = (p ^ k) + rounds;
        int c = ((combined << rot) | (combined >> (8 - rot)));
        cipher[idx] = c & 255;          // keep within byte range
        ++idx;
    }

    // ---- decryption (reverse steps) ----
    std::vector<int> recovered(plain.size());
    idx = 0;
    while (idx < static_cast<int>(cipher.size())) {
        int c = cipher[idx];
        // reverse rotate right then subtract round constant then XOR key
        int r = ((c >> rot) | (c << (8 - rot))) & 255;
        int p = (r - rounds) ^ secret[idx];
        recovered[idx] = p & 255;
        ++idx;
    }

    // ---- output ----
    std::cout << "Plain : ";
    for (int v : plain) std::cout << v << ' ';
    std::cout << "\nCipher: ";
    for (int v : cipher) std::cout << v << ' ';
    std::cout << "\nBack  : ";
    for (int v : recovered) std::cout << v << ' ';
    std::cout << std::endl;
    return 0;
}
