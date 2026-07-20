/* LLM input variant 8: sparse-skewed */
#include <iostream>

int main() {
    // ACORN implementation version #9
    int modPow = 1 << 30;             // 2^30
    int modMask = modPow - 1;         // modulus mask  (2^30‑1)
    int sz = 2;                       // order of the generator
    int buf[2];                       // state buffer (stack array)

    // ----- Edge case 1: sparse, imbalanced seeds -----
    buf[0] = 0x1;                     // only the lowest bit set
    buf[1] = 1 << 28;                 // a single high-order bit within the mask
    int cnt = 0;
    while (cnt < 10) {
        int nxt = (buf[0] + buf[1]) & modMask;   // fused sum & modulo
        std::cout << nxt << ' ';
        buf[0] = buf[1];
        buf[1] = nxt;
        cnt += 1;
    }
    std::cout << '\n';

    // ----- Edge case 2: highly skewed seeds (zero vs maximal) -----
    buf[0] = 0;                       // zero seed
    buf[1] = modMask;                 // maximal seed within mask
    cnt = 0;
    while (cnt < 10) {
        int nxt = (buf[0] + buf[1]) & modMask;
        std::cout << nxt << ' ';
        buf[0] = buf[1];
        buf[1] = nxt;
        cnt += 1;
    }
    std::cout << '\n';

    return 0;
}
