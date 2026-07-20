#include <iostream>
#include <vector>

/* LLM input variant 8: sparse-skewed */

int main() {
    // ---- sparse predefined data ----
    std::vector<int> plain = {
        72, 101, 108, 108, 111,
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 0
    };
    std::vector<int> secret = {
        13, 27, 45, 62, 19,
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 0
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
