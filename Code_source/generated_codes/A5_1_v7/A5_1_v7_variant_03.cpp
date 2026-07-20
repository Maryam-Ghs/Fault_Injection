/*
 * LLM input variant 3: zeros-and-ones
 *
 * A5/1 stream cipher – version #7
 * -------------------------------------------------
 * This implementation follows the classic A5/1 design
 * but is written from scratch with the following
 * constraints:
 *   • Only int and float types are used (no double, long,
 *     unsigned or const)
 *   • All registers are allocated on the heap (new / delete)
 *   • The code is split into small helper functions
 *   • Verbose, step‑by‑step style with reordered arithmetic
 *   • Edge‑case inputs are generated internally (all‑ones key
 *     and frame number)
 * -------------------------------------------------
 */

#include <iostream>
#include <iomanip>

/* Helper to compute XOR of two bits using only +, -, * and &
 *   a XOR b  ==  (a + b) - 2*(a & b)
 */
int bit_xor(int a, int b)
{
    int sum = a + b;
    int and_part = a & b;
    int result = sum - (and_part << 1);   // (a + b) - 2*(a & b)
    return result;
}

/* Compute XOR of an arbitrary number of tapped bits.
 * taps[] holds the indices of the tap positions.
 */
int xor_taps(int *reg, int *taps, int tap_cnt)
{
    int acc = 0;                     // start with 0 (neutral element)
    int i = 0;
    while (i < tap_cnt)
    {
        int tap_val = reg[taps[i]];
        acc = bit_xor(acc, tap_val); // reorder: acc = (acc + tap_val) - 2*(acc & tap_val)
        i = i + 1;                    // manual increment (no ++)
    }
    return acc;
}

/* Shift a register to the right and insert new_bit at position 0 */
void shift_register(int *reg, int len, int new_bit)
{
    int idx = len - 1;
    while (idx > 0)
    {
        reg[idx] = reg[idx - 1];
        idx = idx - 1;
    }
    reg[0] = new_bit;
}

/* Clock a single LFSR using its tap definition */
void clock_lfsr(int *reg, int len, int *taps, int tap_cnt, int external_bit)
{
    int fb = xor_taps(reg, taps, tap_cnt);
    int new_bit = bit_xor(fb, external_bit);   // XOR feedback with external influence
    shift_register(reg, len, new_bit);
}

/* Majority function for three control bits */
int majority(int a, int b, int c)
{
    int sum = a + b + c;          // sum can be 0..3
    int maj = (sum >= 2) ? 1 : 0;
    return maj;
}

/* Load a sequence of bits (all‑ones) into the three registers.
 * The bits are supplied via the array bit_seq[] of given size.
 */
void load_bits(int *R1, int *R2, int *R3,
               int *taps1, int tap1_cnt,
               int *taps2, int tap2_cnt,
               int *taps3, int tap3_cnt,
               int *bit_seq, int seq_len)
{
    int pos = 0;
    while (pos < seq_len)
    {
        int cur_bit = bit_seq[pos];
        clock_lfsr(R1, 19, taps1, tap1_cnt, cur_bit);
        clock_lfsr(R2, 22, taps2, tap2_cnt, cur_bit);
        clock_lfsr(R3, 23, taps3, tap3_cnt, cur_bit);
        pos = pos + 1;
    }
}

/* Warm‑up phase: 100 cycles of majority‑controlled clocking */
void warmup(int *R1, int *R2, int *R3,
            int *taps1, int tap1_cnt,
            int *taps2, int tap2_cnt,
            int *taps3, int tap3_cnt)
{
    int cycle = 0;
    while (cycle < 100)
    {
        int ctrl1 = R1[8];
        int ctrl2 = R2[10];
        int ctrl3 = R3[10];
        int maj = majority(ctrl1, ctrl2, ctrl3);

        if (ctrl1 == maj) clock_lfsr(R1, 19, taps1, tap1_cnt, 0);
        if (ctrl2 == maj) clock_lfsr(R2, 22, taps2, tap2_cnt, 0);
        if (ctrl3 == maj) clock_lfsr(R3, 23, taps3, tap3_cnt, 0);

        cycle = cycle + 1;
    }
}

/* Generate N keystream bits after warm‑up */
void generate_keystream(int *R1, int *R2, int *R3,
                        int *taps1, int tap1_cnt,
                        int *taps2, int tap2_cnt,
                        int *taps3, int tap3_cnt,
                        int *output, int n_bits)
{
    int produced = 0;
    while (produced < n_bits)
    {
        int ctrl1 = R1[8];
        int ctrl2 = R2[10];
        int ctrl3 = R3[10];
        int maj = majority(ctrl1, ctrl2, ctrl3);

        if (ctrl1 == maj) clock_lfsr(R1, 19, taps1, tap1_cnt, 0);
        if (ctrl2 == maj) clock_lfsr(R2, 22, taps2, tap2_cnt, 0);
        if (ctrl3 == maj) clock_lfsr(R3, 23, taps3, tap3_cnt, 0);

        /* Output bit = XOR of the last bits of each register */
        int out1 = R1[18];
        int out2 = R2[21];
        int out3 = R3[22];
        int mix12 = bit_xor(out1, out2);
        int keystream_bit = bit_xor(mix12, out3);
        output[produced] = keystream_bit;

        produced = produced + 1;
    }
}

/* ----------------------------------------------------------------- */
/* Main entry point – generates and prints a 10‑bit keystream        */
/* ----------------------------------------------------------------- */
int main()
{
    /* -----------------------------------------------------------------
     * 1. Allocate three shift registers on the heap
     * ----------------------------------------------------------------- */
    int *R1 = new int[19];
    int *R2 = new int[22];
    int *R3 = new int[23];

    /* Initialise all cells to 0 */
    int i = 0;
    while (i < 19) { R1[i] = 0; i = i + 1; }
    i = 0;
    while (i < 22) { R2[i] = 0; i = i + 1; }
    i = 0;
    while (i < 23) { R3[i] = 0; i = i + 1; }

    /* -----------------------------------------------------------------
     * 2. Define tap positions (zero‑based indices)
     * ----------------------------------------------------------------- */
    int taps_R1[] = {13, 16, 17, 18};      // 19‑bit LFSR
    int taps_R2[] = {20, 21};              // 22‑bit LFSR
    int taps_R3[] = {7, 20, 21, 22};       // 23‑bit LFSR

    /* -----------------------------------------------------------------
     * 3. Prepare edge‑case inputs:
     *    - key: all zeros (64 bits)
     *    - frame: alternating zeros and ones (22 bits)
     * ----------------------------------------------------------------- */
    int key_bits[64];
    int frame_bits[22];
    i = 0;
    while (i < 64) { key_bits[i] = 0; i = i + 1; }          // every bit = 0
    i = 0;
    while (i < 22) { frame_bits[i] = i % 2; i = i + 1; }   // 0,1,0,1,...

    /* -----------------------------------------------------------------
     * 4. Load key then frame into the registers (XOR‑style loading)
     * ----------------------------------------------------------------- */
    load_bits(R1, R2, R3,
              taps_R1, 4,
              taps_R2, 2,
              taps_R3, 4,
              key_bits, 64);

    load_bits(R1, R2, R3,
              taps_R1, 4,
              taps_R2, 2,
              taps_R3, 4,
              frame_bits, 22);

    /* -----------------------------------------------------------------
     * 5. Warm‑up phase (100 majority‑controlled cycles)
     * ----------------------------------------------------------------- */
    warmup(R1, R2, R3,
           taps_R1, 4,
           taps_R2, 2,
           taps_R3, 4);

    /* -----------------------------------------------------------------
     * 6. Generate a short keystream (10 bits) and print it
     * ----------------------------------------------------------------- */
    const int KS_LEN = 10;                 // length of keystream to display
    int *keystream = new int[KS_LEN];

    generate_keystream(R1, R2, R3,
                       taps_R1, 4,
                       taps_R2, 2,
                       taps_R3, 4,
                       keystream, KS_LEN);

    std::cout << "A5/1 keystream (10 bits, zero key & alternating frame): ";
    i = 0;
    while (i < KS_LEN)
    {
        std::cout << keystream[i];
        i = i + 1;
    }
    std::cout << std::endl;

    /* -----------------------------------------------------------------
     * 7. Clean up heap memory
     * ----------------------------------------------------------------- */
    delete [] R1;
    delete [] R2;
    delete [] R3;
    delete [] keystream;

    return 0;
}
