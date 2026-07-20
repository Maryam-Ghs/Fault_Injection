#include <iostream>

/* LLM input variant 2: small-diverse */

int main() {
    // ACORN implementation version #9 (variant)
    int modPow = 1 << 12;                 // 2^12 = 4096
    int modMask = modPow - 1;            // modulus mask (4095)
    int sz = 3;                           // order of the generator
    int buf[3];                            // state buffer (stack array)

    // ----- Variant 1: all zero seeds -----
    buf[0] = 0;
    buf[1] = 0;
    buf[2] = 0;
    int cnt = 0;
    while (cnt < 10) {
        int nxt = (buf[0] + buf[1] + buf[2]) & modMask; // fused sum & modulo
        std::cout << nxt << ' ';
        buf[0] = buf[1];
        buf[1] = buf[2];
        buf[2] = nxt;
        cnt += 1;
    }
    std::cout << '\n';

    // ----- Variant 2: maximal seeds (modMask) -----
    buf[0] = modMask;
    buf[1] = modMask;
    buf[2] = modMask;
    cnt = 0;
    while (cnt < 10) {
        int nxt = (buf[0] + buf[1] + buf[2]) & modMask;
        std::cout << nxt << ' ';
        buf[0] = buf[1];
        buf[1] = buf[2];
        buf[2] = nxt;
        cnt += 1;
    }
    std::cout << '\n';

    return 0;
}
