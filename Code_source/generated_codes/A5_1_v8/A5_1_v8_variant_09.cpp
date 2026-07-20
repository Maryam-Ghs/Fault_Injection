#include <iostream>
#include <cstdlib>

/* LLM input variant 9: medium-deterministic-random */

// ------------------------------------------------------------
// Helper: compute majority of three bits (0 or 1)
int maj(int a, int b, int c)
{
    int sum = a + b + c;
    return (sum >= 2) ? 1 : 0;
}

// Helper: initialise a register with deterministic bits (0/1)
void fill_reg(int *reg, int sz)
{
    for (int i = 0; i < sz; ++i)
    {
        // Simple deterministic pattern: (i * 7 + 3) mod 2
        reg[i] = ((i * 7 + 3) % 2);
    }
}

// Helper: compute feedback from tapped positions and shift left
void clock_one(int *reg, int sz, int *taps, int tap_cnt)
{
    // compute XOR of tapped bits
    int fb = 0;
    for (int j = 0; j < tap_cnt; ++j)
    {
        fb ^= reg[taps[j]];
    }

    // shift all bits towards the end (right side)
    for (int k = sz - 1; k > 0; --k)
    {
        reg[k] = reg[k - 1];
    }
    // insert feedback at the beginning (position 0)
    reg[0] = fb;
}

// ------------------------------------------------------------
int main()
{
    // Fixed deterministic seed
    std::srand(123456789);

    // lengths of the three LFSRs
    int len_a = 19;
    int len_b = 22;
    int len_c = 23;

    // allocate registers on the heap
    int *reg_a = new int[len_a];
    int *reg_b = new int[len_b];
    int *reg_c = new int[len_c];

    // fill registers with deterministic start values
    fill_reg(reg_a, len_a);
    fill_reg(reg_b, len_b);
    fill_reg(reg_c, len_c);

    // tap positions for each register (zero‑based)
    // R1 taps: 13, 16, 17, 18
    int taps_a[] = {13, 16, 17, 18};
    int tapcnt_a = 4;
    // R2 taps: 20, 21
    int taps_b[] = {20, 21};
    int tapcnt_b = 2;
    // R3 taps: 7, 20, 21, 22
    int taps_c[] = {7, 20, 21, 22};
    int tapcnt_c = 4;

    // clocking bit positions (zero‑based)
    int clk_a = 8;
    int clk_b = 10;
    int clk_c = 10;

    // number of keystream bits to produce (medium size)
    int out_len = 128;
    int *keystream = new int[out_len];

    int out_idx = 0;
    while (out_idx < out_len)
    {
        // read the clocking bits
        int bit_a = reg_a[clk_a];
        int bit_b = reg_b[clk_b];
        int bit_c = reg_c[clk_c];

        // decide which registers are clocked
        int maj_bit = maj(bit_a, bit_b, bit_c);

        if (bit_a == maj_bit) clock_one(reg_a, len_a, taps_a, tapcnt_a);
        if (bit_b == maj_bit) clock_one(reg_b, len_b, taps_b, tapcnt_b);
        if (bit_c == maj_bit) clock_one(reg_c, len_c, taps_c, tapcnt_c);

        // generate one keystream bit (XOR of the output bits)
        int ks = reg_a[0] ^ reg_b[0] ^ reg_c[0];
        keystream[out_idx] = ks;

        ++out_idx;
    }

    // print the resulting keystream
    std::cout << "A5/1 keystream (" << out_len << " bits):\n";
    for (int p = 0; p < out_len; ++p)
    {
        std::cout << keystream[p];
        if ((p + 1) % 8 == 0) std::cout << ' ';
    }
    std::cout << std::endl;

    // clean up heap memory
    delete[] reg_a;
    delete[] reg_b;
    delete[] reg_c;
    delete[] keystream;

    return 0;
}
