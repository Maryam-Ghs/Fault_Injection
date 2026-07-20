/* LLM input variant 1: minimal-boundary */
#include <iostream>

int main() {
    // ACORN implementation version #9
    int modPow = 1 << 1;              // 2^1 (smallest nontrivial modulus)
    int modMask = modPow - 1;         // modulus mask (2^1‑1) = 1
    int sz = 2;                       // order of the generator
    int buf[2];                       // state buffer (stack array)

    // ----- Edge case 1: all zero seeds -----
    buf[0] = 0;
    buf[1] = 0;
    int cnt = 0;
    while (cnt < 1) {                 // minimal nontrivial iteration count
        int nxt = (buf[0] + buf[1]) & modMask;   // fused sum & modulo
        std::cout << nxt << ' ';
        buf[0] = buf[1];
        buf[1] = nxt;
        cnt += 1;
    }
    std::cout << '\n';

    // ----- Edge case 2: maximal seeds (modMask) -----
    buf[0] = modMask;
    buf[1] = modMask;
    cnt = 0;
    while (cnt < 1) {                 // minimal nontrivial iteration count
        int nxt = (buf[0] + buf[1]) & modMask;
        std::cout << nxt << ' ';
        buf[0] = buf[1];
        buf[1] = nxt;
        cnt += 1;
    }
    std::cout << '\n';

    return 0;
}
