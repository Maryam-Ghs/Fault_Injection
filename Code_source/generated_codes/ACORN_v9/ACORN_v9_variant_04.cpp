/* LLM input variant 4: signed-extremes */
#include <iostream>

int main() {
    // ACORN implementation version #9
    int modPow = 1 << 30;             // 2^30
    int modMask = modPow - 1;         // modulus mask  (2^30‑1)
    int sz = 2;                       // order of the generator
    int buf[2];                       // state buffer (stack array)

    // ----- Edge case 1: mixed negative, zero, and positive seeds -----
    buf[0] = -123456;                 // negative seed
    buf[1] = 0;                       // zero seed
    int cnt = 0;
    while (cnt < 10) {
        int nxt = (buf[0] + buf[1]) & modMask;   // fused sum & modulo
        std::cout << nxt << ' ';
        buf[0] = buf[1];
        buf[1] = nxt;
        cnt += 1;
    }
    std::cout << '\n';

    // ----- Edge case 2: mixed positive and negative maximal‑magnitude seeds -----
    buf[0] = modMask / 2;            // large positive seed
    buf[1] = -(modMask / 2);          // large negative seed, sum zero
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
