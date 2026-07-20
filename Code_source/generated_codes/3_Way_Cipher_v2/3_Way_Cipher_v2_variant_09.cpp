#include <iostream>
#include <vector>

/* LLM input variant 9: medium-deterministic-random */

int main() {
    // ---- medium deterministic pseudo‑random data ----
    std::vector<int> plain = {
        12, 87, 45, 200, 33, 150, 9, 66, 123, 255,
        0, 78, 190, 34, 111, 222, 56, 144, 67, 89,
        101, 210, 3, 175, 60, 99, 140, 27, 184, 5
    };
    std::vector<int> secret = {
        205, 34, 190, 12, 76, 189, 44, 2, 133, 88,
        255, 13, 57, 221, 78, 9, 160, 31, 102, 214,
        50, 173, 84, 6, 199, 120, 33, 147, 91, 64
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
        int c = (((p ^ k) + rounds) << rot) | ((((p ^ k) + rounds) >> (8 - rot)));
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
