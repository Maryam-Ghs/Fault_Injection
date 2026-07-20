#include <iostream>
#include <iomanip>

/* LLM input variant 2: small-diverse */

class AcornV5 {
    int k;                 // order of the generator
    int buf[5];            // internal state (stack array)
public:
    // constructor: copies the seed array into buf
    AcornV5(int order, int seed[]) {
        k = order;
        int p = 0;
        while (p < k) {
            buf[p] = seed[p];
            p = p + 1;
        }
    }

    // produce one 31‑bit integer
    int fetch() {
        // ---- expanded multi‑step sum ----
        int part0 = buf[0] + buf[1];
        int part1 = buf[2] + buf[3];
        int part2 = part0 + part1;
        int sum   = part2 + buf[4];
        // keep only the low 31 bits (branch‑free)
        int fresh = sum & 0x7fffffff;

        // ---- shift right, insert new value on the left ----
        int q = k - 1;
        while (q > 0) {
            buf[q] = buf[q - 1];
            q = q - 1;
        }
        buf[0] = fresh;
        return fresh;
    }
};

int main() {
    // small predefined seed array (stack) with varied values
    int seedArr[5] = { -1, 0, 42, 9999, 123456 };
    AcornV5 rng(5, seedArr);

    int total = 8;   // how many numbers to generate
    int i = 0;
    while (i < total) {
        int raw = rng.fetch();                 // 31‑bit integer
        // convert to float in [0,1)
        float prob = (float)raw / 2147483648.0f;   // 2^31 as float
        std::cout << std::fixed << std::setprecision(6) << prob << '\n';
        i = i + 1;
    }
    return 0;
}
