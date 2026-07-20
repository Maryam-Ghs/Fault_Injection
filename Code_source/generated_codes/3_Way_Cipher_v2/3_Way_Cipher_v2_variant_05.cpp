#include <iostream>
#include <vector>

/* LLM input variant 5: duplicate-heavy */

int main() {
    // ---- small predefined data ----
    std::vector<int> plain = { 65, 65, 65, 66, 66, 66, 66, 65, 65, 66, 66 };
    std::vector<int> secret = { 1, 1, 1, 2, 2, 2, 2, 1, 1, 2, 2 };
    const int rounds = 3;               // allowed as literal, not a variable
    const int rot = 3;                  // rotation amount

    // ---- encryption ----
    std::vector<int> cipher(plain.size());
    int idx = 0;
    while (idx < static_cast<int>(plain.size())) {
        int p = plain[idx];
        int k = secret[idx];
        // fused three‑way transformation: XOR, add round constant, rotate left
        int c = (( (p ^ k) + rounds ) << rot) | ( ( (p ^ k) + rounds ) >> (8 - rot) );
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
