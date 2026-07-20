/*********************************************************************
 * AES‑128‑CFB implementation – version #5
 * ---------------------------------------------------------------
 * Input variant: zeros-and-ones
 * ---------------------------------------------------------------
 * All code lives inside main().  Only int (and float, not used) are
 * employed – no unsigned, double, long or const qualifiers.
 * Random data generation is replaced with deterministic zero/one patterns.
 *********************************************************************/

#include <iostream>
#include <iomanip>

/* LLM input variant 3: zeros-and-ones */

int main()
{
    /******************** 1. Helper: simple pseudo‑random generator **********/
    int seed = 1234567;                     // any non‑zero start value
    auto rnd = [&seed]() -> int
    {
        // Linear Congruential Generator:  a = 1103515245, c = 12345, m = 2^31
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        return seed & 0xff;                // keep only a byte
    };

    /******************** 2. Fixed tables (S‑box, Rcon) ********************/
    int sbox[256] = {
        99,124,119,123,242,107,111,197,48,1,103,43,254,215,171,118,
        202,130,201,125,250,89,71,240,173,212,162,175,156,164,114,192,
        183,253,147,38,54,63,247,204,52,165,229,241,113,216,49,21,
        4,199,35,195,24,150,5,154,7,18,128,226,235,39,178,117,
        9,131,44,26,27,110,90,160,82,59,214,179,41,227,47,132,
        83,209,0,237,32,252,177,91,106,203,190,57,74,76,88,207,
        208,239,170,251,67,77,51,133,69,249,2,127,80,60,159,168,
        81,163,64,143,146,157,56,245,188,182,218,33,16,255,243,210,
        205,12,19,236,95,151,68,23,196,167,126,61,100,93,25,115,
        96,129,79,220,34,42,144,136,70,238,184,20,222,94,11,219,
        224,50,58,10,73,6,36,92,194,211,172,98,145,149,228,121,
        231,200,55,109,141,213,78,169,108,86,244,234,101,122,174,8,
        186,120,37,46,28,166,180,198,232,221,116,31,75,189,139,138,
        112,62,181,102,72,3,246,14,97,53,87,185,134,193,29,158,
        225,248,152,17,105,217,142,148,155,30,135,233,206,85,40,223,
        140,161,137,13,191,230,66,104,65,153,45,15,176,84,187,22
    };

    int rcon[11] = {0,1,2,4,8,16,32,64,128,27,54};

    /******************** 3. Key, IV and plaintext generation **************/
    int key[16];
    int iv[16];
    int plain[64];                         // 4 blocks (64 bytes)

    // Deterministic zero/one patterns
    for (int i = 0; i < 16; ++i) key[i] = i % 2;            // 0,1,0,1,...
    for (int i = 0; i < 16; ++i) iv[i] = 0;                 // all zeros
    for (int i = 0; i < 64; ++i) plain[i] = (i % 8 == 0) ? 1 : 0; // a few ones, mostly zeros

    /******************** 4. Key schedule (expand to 11 round keys) ********/
    int roundKey[11][16];
    // copy initial key as round 0
    for (int i = 0; i < 16; ++i) { roundKey[0][i] = key[i]; }

    // helper lambda: rotate a word (4 bytes)
    auto rotWord = [](int* w)
    {
        int t = w[0];
        w[0] = w[1]; w[1] = w[2]; w[2] = w[3]; w[3] = t;
    };

    // helper lambda: apply S‑box to a word
    auto subWord = [&](int* w)
    {
        for (int i = 0; i < 4; ++i) w[i] = sbox[w[i]];
    };

    // expand
    for (int round = 1; round <= 10; ++round)
    {
        int temp[4];
        // take last word of previous round
        for (int i = 0; i < 4; ++i) temp[i] = roundKey[round-1][12 + i];
        rotWord(temp);
        subWord(temp);
        temp[0] ^= rcon[round];                // Rcon applied only to first byte

        // first word of this round
        for (int i = 0; i < 4; ++i)
            roundKey[round][i] = roundKey[round-1][i] ^ temp[i];

        // remaining three words
        for (int w = 1; w < 4; ++w)
        {
            for (int i = 0; i < 4; ++i)
                roundKey[round][4*w + i] = roundKey[round-1][4*w + i] ^ roundKey[round][4*(w-1) + i];
        }
    }

    /******************** 5. Core AES round functions **********************/
    // SubBytes
    auto subBytes = [&](int* state)
    {
        for (int i = 0; i < 16; ++i) state[i] = sbox[state[i]];
    };

    // ShiftRows (in‑place)
    auto shiftRows = [&](int* st)
    {
        int tmp[16];
        // row 0 (no shift)
        tmp[0]  = st[0];  tmp[4]  = st[4];  tmp[8]  = st[8];  tmp[12] = st[12];
        // row 1 (shift left by 1)
        tmp[1]  = st[5];  tmp[5]  = st[9];  tmp[9]  = st[13]; tmp[13] = st[1];
        // row 2 (shift left by 2)
        tmp[2]  = st[10]; tmp[6]  = st[14]; tmp[10] = st[2];  tmp[14] = st[6];
        // row 3 (shift left by 3)
        tmp[3]  = st[15]; tmp[7]  = st[3];  tmp[11] = st[7];  tmp[15] = st[11];
        for (int i = 0; i < 16; ++i) st[i] = tmp[i];
    };

    // MixColumns (in‑place, using integer arithmetic)
    auto mixColumns = [&](int* st)
    {
        for (int c = 0; c < 4; ++c)
        {
            int i0 = 4*c, i1 = i0+1, i2 = i0+2, i3 = i0+3;
            int a0 = st[i0], a1 = st[i1], a2 = st[i2], a3 = st[i3];
            // multiplication in GF(2^8) with 0x02 and 0x03
            auto xtime = [&](int x){ int r = x << 1; if (x & 0x80) r ^= 0x1b; return r & 0xff; };
            int r0 = xtime(a0) ^ (xtime(a1) ^ a1) ^ a2 ^ a3;
            int r1 = a0 ^ xtime(a1) ^ (xtime(a2) ^ a2) ^ a3;
            int r2 = a0 ^ a1 ^ xtime(a2) ^ (xtime(a3) ^ a3);
            int r3 = (xtime(a0) ^ a0) ^ a1 ^ a2 ^ xtime(a3);
            st[i0] = r0; st[i1] = r1; st[i2] = r2; st[i3] = r3;
        }
    };

    // AddRoundKey (in‑place)
    auto addRoundKey = [&](int* st, int round)
    {
        for (int i = 0; i < 16; ++i) st[i] ^= roundKey[round][i];
    };

    // Full AES encryption of one 16‑byte block
    auto aesEncryptBlock = [&](int* block)
    {
        int state[16];
        for (int i = 0; i < 16; ++i) state[i] = block[i];

        addRoundKey(state,0);
        for (int r = 1; r <= 9; ++r)
        {
            subBytes(state);
            shiftRows(state);
            mixColumns(state);
            addRoundKey(state,r);
        }
        subBytes(state);
        shiftRows(state);
        addRoundKey(state,10);

        for (int i = 0; i < 16; ++i) block[i] = state[i];
    };

    /******************** 6. CFB‑128 encryption ****************************/
    int ciphertext[64];
    int feedback[16];
    // initialise feedback with IV
    for (int i = 0; i < 16; ++i) feedback[i] = iv[i];

    // process each 16‑byte segment
    for (int offset = 0; offset < 64; offset += 16)
    {
        int encrypted[16];
        for (int i = 0; i < 16; ++i) encrypted[i] = feedback[i];
        aesEncryptBlock(encrypted);                     // encrypt feedback

        // XOR with plaintext to obtain ciphertext segment
        for (int i = 0; i < 16; ++i)
        {
            ciphertext[offset + i] = encrypted[i] ^ plain[offset + i];
            feedback[i] = ciphertext[offset + i];       // next feedback = ciphertext block
        }
    }

    /******************** 7. Output ****************************************/
    std::cout << "Plaintext : ";
    for (int i = 0; i < 64; ++i)
        std::cout << std::hex << std::setw(2) << std::setfill('0') << plain[i];
    std::cout << std::dec << "\n";

    std::cout << "Ciphertext: ";
    for (int i = 0; i < 64; ++i)
        std::cout << std::hex << std::setw(2) << std::setfill('0') << ciphertext[i];
    std::cout << std::dec << "\n";

    return 0;
}
