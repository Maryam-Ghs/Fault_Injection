#include <iostream>
#include <vector>

/* LLM input variant 10: large-safe-stress */

int main() {
    // ---- large predefined data ----
    const int N = 5000; // relatively large but safe size
    std::vector<int> plain(N);
    std::vector<int> secret(N);
    for (int i = 0; i < N; ++i) {
        plain[i] = (i * 31) % 256;
        secret[i] = (i * 47 + 13) % 256;
    }
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
