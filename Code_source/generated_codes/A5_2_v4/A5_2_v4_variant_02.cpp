#include <iostream>
#include <cstdlib>
#include <ctime>

/* LLM input variant 2: small-diverse */

// ---------- helper functions -------------------------------------------------

// compute XOR of tapped bits (order of operations deliberately shuffled)
int feedback_calc(int *reg, int sz, int *taps, int tcnt)
{
    int tmp = 0;
    for (int i = 0; i < tcnt; ++i)
    {
        // reorder: XOR first with the last tap, then with the rest
        tmp = tmp ^ reg[taps[tcnt - 1 - i]];
    }
    // final re‑order step
    tmp = tmp ^ reg[taps[0]];
    return tmp & 1;
}

// shift register to the right, insert new bit at position 0
void shift_reg(int *reg, int sz, int newbit)
{
    for (int i = sz - 1; i > 0; --i)
        reg[i] = reg[i - 1];
    reg[0] = newbit & 1;
}

// majority of three bits (loop‑heavy version)
int maj_bit(int a, int b, int c)
{
    int sum = 0;
    for (int i = 0; i < 3; ++i)
    {
        int cur = (i == 0) ? a : ((i == 1) ? b : c);
        sum = sum + cur;               // addition first
    }
    // reorder: compare with 2 after subtraction
    return ((sum - 2) >= 0) ? 1 : 0;
}

// clock the four registers according to A5/2‑like rule
void clock_all(int *r1, int *r2, int *r3, int *r4,
               int sz1, int sz2, int sz3, int sz4,
               int *tap1, int t1c,
               int *tap2, int t2c,
               int *tap3, int t3c,
               int *tap4, int t4c)
{
    // control bits
    int ctrl1 = r1[8 % sz1];
    int ctrl2 = r2[10 % sz2];
    int ctrl3 = r3[10 % sz3];
    int maj   = maj_bit(ctrl1, ctrl2, ctrl3);

    // decide which registers to clock
    if (ctrl1 == maj)
        shift_reg(r1, sz1, feedback_calc(r1, sz1, tap1, t1c));
    if (ctrl2 == maj)
        shift_reg(r2, sz2, feedback_calc(r2, sz2, tap2, t2c));
    if (ctrl3 == maj)
        shift_reg(r3, sz3, feedback_calc(r3, sz3, tap3, t3c));

    // R4 is always clocked
    shift_reg(r4, sz4, feedback_calc(r4, sz4, tap4, t4c));
}

// generate keystream of given length
void gen_stream(int *out, int outlen,
                int *r1, int *r2, int *r3, int *r4,
                int sz1, int sz2, int sz3, int sz4,
                int *tap1, int t1c,
                int *tap2, int t2c,
                int *tap3, int t3c,
                int *tap4, int t4c)
{
    for (int i = 0; i < outlen; ++i)
    {
        // output bit is XOR of the first bits of all registers
        int ksbit = (r1[0] ^ r2[0]) ^ (r3[0] ^ r4[0]);
        out[i] = ksbit & 1;

        // clock registers for next round
        clock_all(r1, r2, r3, r4,
                  sz1, sz2, sz3, sz4,
                  tap1, t1c,
                  tap2, t2c,
                  tap3, t3c,
                  tap4, t4c);
    }
}

// deterministic fill of a register with a simple pattern
void deterministic_fill(int *reg, int sz)
{
    for (int i = 0; i < sz; ++i)
        reg[i] = ((i * 3) + 1) % 2; // yields a repeating 1,0,1,0,...
}

// ---------- main -------------------------------------------------------------

int main()
{
    // lengths of the four LFSRs (small and varied)
    int len1 = 13, len2 = 15, len3 = 20, len4 = 21;

    // allocate registers on the heap
    int *regA = new int[len1];
    int *regB = new int[len2];
    int *regC = new int[len3];
    int *regD = new int[len4];

    // deterministic initialisation (acts as key + frame number)
    deterministic_fill(regA, len1);
    deterministic_fill(regB, len2);
    deterministic_fill(regC, len3);
    deterministic_fill(regD, len4);

    // tap positions for each register (zero‑based)
    int tapsA[3] = {0, 2, 5};
    int tapsB[4] = {0, 1, 4, 7};
    int tapsC[4] = {0, 3, 6, 9};
    int tapsD[5] = {0, 2, 5, 11, 16};

    // produce 64 keystream bits (small yet diverse)
    const int KS_LEN = 64;
    int *keystream = new int[KS_LEN];

    gen_stream(keystream, KS_LEN,
               regA, regB, regC, regD,
               len1, len2, len3, len4,
               tapsA, 3,
               tapsB, 4,
               tapsC, 4,
               tapsD, 5);

    // output the keystream
    for (int i = 0; i < KS_LEN; ++i)
        std::cout << keystream[i];
    std::cout << std::endl;

    // clean up
    delete [] regA;
    delete [] regB;
    delete [] regC;
    delete [] regD;
    delete [] keystream;

    return 0;
}
