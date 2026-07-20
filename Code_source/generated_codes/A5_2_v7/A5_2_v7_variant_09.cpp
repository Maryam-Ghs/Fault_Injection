#include <iostream>
#include <iomanip>

/* LLM input variant 9: medium-deterministic-random */

// ------------------------------------------------------------
// Helper functions
// ------------------------------------------------------------
int maj(int x, int y, int z)            // majority of three bits
{
    int sum = x + y + z;
    return (sum >= 2) ? 1 : 0;
}

// simple LFSR step, taps are given as an array of positions (0‑based)
void lfsr_step(int *reg, int reg_len, const int *tap_pos, int tap_cnt)
{
    int new_bit = 0;
    for (int i = 0; i < tap_cnt; ++i)
        new_bit ^= reg[tap_pos[i]];
    // shift right
    for (int i = reg_len - 1; i > 0; --i)
        reg[i] = reg[i - 1];
    reg[0] = new_bit;
}

// clock the three data registers according to the control register
void irregular_clock(int *regA, int *regB, int *regC,
                     const int *tapA, int cntA,
                     const int *tapB, int cntB,
                     const int *tapC, int cntC,
                     int *ctrlReg, const int *tapCtrl, int cntCtrl)
{
    // control bits used for majority decision
    int ca = ctrlReg[3];
    int cb = ctrlReg[7];
    int cc = ctrlReg[10];
    int m  = maj(ca, cb, cc);

    // clock each data register if its control bit equals the majority
    if (ca == m) lfsr_step(regA, 19, tapA, cntA);
    if (cb == m) lfsr_step(regB, 22, tapB, cntB);
    if (cc == m) lfsr_step(regC, 23, tapC, cntC);

    // always clock the control register
    lfsr_step(const_cast<int*>(ctrlReg), 17, tapCtrl, cntCtrl);
}

// produce one keystream bit
int keystream_bit(int *regA, int *regB, int *regC)
{
    int out = (regA[18] ^ regB[21] ^ regC[22]) & 1;
    return out;
}

// ------------------------------------------------------------
// Main routine
// ------------------------------------------------------------
int main()
{
    // --------------------------------------------------------
    // 1. Allocate registers on the heap
    // --------------------------------------------------------
    int *regA = new int[19];   // R1
    int *regB = new int[22];   // R2
    int *regC = new int[23];   // R3
    int *ctrlReg = new int[17]; // R4 (control)

    // --------------------------------------------------------
    // 2. Define taps (positions are 0‑based, leftmost bit = index 0)
    // --------------------------------------------------------
    const int tapA[4] = {13, 16, 17, 18};            // R1 taps
    const int tapB[2] = {20, 21};                    // R2 taps
    const int tapC[4] = {7, 20, 21, 22};             // R3 taps
    const int tapCtrl[6] = {0, 3, 7, 10, 14, 16};    // R4 taps

    // --------------------------------------------------------
    // 3. Deterministic test vectors (key = 64 bits, frame = 22 bits)
    // --------------------------------------------------------
    int keyBits[64];
    int frameBits[22];

    // deterministic pseudo‑random pattern: linear congruential (mod 2)
    for (int i = 0; i < 64; ++i)
        keyBits[i] = ((i * 7 + 3) % 2);
    for (int i = 0; i < 22; ++i)
        frameBits[i] = ((i * 5 + 2) % 2);

    // --------------------------------------------------------
    // 4. Initialise all registers to zero
    // --------------------------------------------------------
    for (int i = 0; i < 19; ++i) regA[i] = 0;
    for (int i = 0; i < 22; ++i) regB[i] = 0;
    for (int i = 0; i < 23; ++i) regC[i] = 0;
    for (int i = 0; i < 17; ++i) ctrlReg[i] = 0;

    // --------------------------------------------------------
    // 5. Load the 64‑bit key (first 19 bits -> R1, next 22 -> R2,
    //    remaining 23 -> R3)
    // --------------------------------------------------------
    for (int i = 0; i < 19; ++i) regA[i] = keyBits[i];
    for (int i = 0; i < 22; ++i) regB[i] = keyBits[19 + i];
    for (int i = 0; i < 23; ++i) regC[i] = keyBits[41 + i];

    // --------------------------------------------------------
    // 6. Load the 22‑bit frame number into the control register
    //    (bits are placed starting at position 0)
    // --------------------------------------------------------
    for (int i = 0; i < 22; ++i)
        ctrlReg[i % 17] ^= frameBits[i];   // simple mixing into R4

    // --------------------------------------------------------
    // 7. Warm‑up phase: run 150 cycles without producing output
    // --------------------------------------------------------
    int warm = 150;
    while (warm--)
        irregular_clock(regA, regB, regC,
                        tapA, 4, tapB, 2, tapC, 4,
                        ctrlReg, tapCtrl, 6);

    // --------------------------------------------------------
    // 8. Generate 228 keystream bits (standard GSM burst length)
    // --------------------------------------------------------
    const int KS_LEN = 228;
    int *keyStream = new int[KS_LEN];
    int idx = 0;
    while (idx < KS_LEN)
    {
        irregular_clock(regA, regB, regC,
                        tapA, 4, tapB, 2, tapC, 4,
                        ctrlReg, tapCtrl, 6);
        keyStream[idx++] = keystream_bit(regA, regB, regC);
    }

    // --------------------------------------------------------
    // 9. Print the keystream as hexadecimal groups (8‑bit chunks)
    // --------------------------------------------------------
    std::cout << "A5/2 keystream (228 bits) for test vector:\n";
    for (int i = 0; i < KS_LEN; i += 8)
    {
        int byte = 0;
        for (int b = 0; b < 8 && (i + b) < KS_LEN; ++b)
            byte = (byte << 1) | keyStream[i + b];
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << byte << " ";
    }
    std::cout << std::dec << std::endl;

    // --------------------------------------------------------
    // 10. Clean up heap memory
    // --------------------------------------------------------
    delete[] regA;
    delete[] regB;
    delete[] regC;
    delete[] ctrlReg;
    delete[] keyStream;

    return 0;
}
