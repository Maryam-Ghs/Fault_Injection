#include <iostream>

/* LLM input variant 7: reverse-adversarial */

int main() {
    // ACORN implementation version #9
    int modPow = 1 << 30;             // 2^30
    int modMask = modPow - 1;         // modulus mask  (2^30‑1)
    int sz = 2;                       // order of the generator
    int buf[2];                       // state buffer (stack array)

    // ----- Edge case 1: reverse maximal and zero seeds -----
    buf[0] = modMask;   // start with maximal seed
    buf[1] = 0;          // second seed zero
    int cnt = 0;
    while (cnt < 20) {  // extended length for adversarial pattern
        int nxt = (buf[0] + buf[1]) & modMask;   // fused sum & modulo
        std::cout << nxt << ' ';
        buf[0] = buf[1];
        buf[1] = nxt;
        cnt += 1;
    }
    std::cout << '\n';

    // ----- Edge case 2: zero then maximal seeds -----
    buf[0] = 0;
    buf[1] = modMask;
    cnt = 0;
    while (cnt < 20) {
        int nxt = (buf[0] + buf[1]) & modMask;
        std::cout << nxt << ' ';
        buf[0] = buf[1];
        buf[1] = nxt;
        cnt += 1;
    }
    std::cout << '\n';

    return 0;
}
