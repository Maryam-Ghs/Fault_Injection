#include <iostream>
#include <iomanip>

/* LLM input variant 9: medium-deterministic-random */

int main()
{
    /* --------------------------------------------------------------------
       1.  Define the AES S‑box and Rcon (all values are stored as signed int)
       -------------------------------------------------------------------- */
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

    int rcon[10] = {1,2,4,8,16,32,64,128,27,54};

    /* --------------------------------------------------------------------
       2.  Prepare deterministic pseudo‑random test vectors (key & plaintext)
       -------------------------------------------------------------------- */
    int *secretKey   = new int[16];
    int *plainBlock  = new int[16];
    unsigned int seedKey = 0x12345678;
    unsigned int seedPlain = 0x9ABCDEF0;
    for (int i = 0; i < 16; ++i) {
        // Simple LCG for reproducible pseudo‑random bytes
        seedKey = (seedKey * 1103515245 + 12345) & 0xFFFFFFFF;
        secretKey[i] = (seedKey >> 24) & 0xFF;

        seedPlain = (seedPlain * 1664525 + 1013904223) & 0xFFFFFFFF;
        plainBlock[i] = (seedPlain >> 16) & 0xFF;
    }

    /* --------------------------------------------------------------------
       3.  Expand the key (44 words -> 11 round keys, each 16 bytes)
       -------------------------------------------------------------------- */
    int *roundKeys = new int[11 * 16];   // roundKeys[round*16 + byte]

    // copy first round key (the original key)
    for (int i = 0; i < 16; ++i) {
        roundKeys[i] = secretKey[i];
    }

    // temporary word for key schedule (4 bytes)
    int temp[4];
    int generatedWords = 4; // we already have 4 words (16 bytes)

    while (generatedWords < 44) {
        // copy previous word
        int srcIdx = (generatedWords - 1) * 4;
        for (int dstIdx = 0; dstIdx < 4; ++dstIdx) {
            temp[dstIdx] = roundKeys[srcIdx + dstIdx];
        }

        // every 4th word undergoes RotWord + SubWord + Rcon
        if (generatedWords % 4 == 0) {
            // RotWord
            int tmp = temp[0];
            temp[0] = temp[1];
            temp[1] = temp[2];
            temp[2] = temp[3];
            temp[3] = tmp;

            // SubWord
            for (int j = 0; j < 4; ++j) {
                temp[j] = sbox[temp[j]];
            }

            // Rcon XOR on first byte
            temp[0] = temp[0] ^ rcon[(generatedWords/4)-1];
        }

        // XOR with word 4 positions earlier
        int prevIdx = (generatedWords - 4) * 4;
        for (int k = 0; k < 4; ++k) {
            temp[k] = temp[k] ^ roundKeys[prevIdx + k];
        }

        // store the new word
        int storeIdx = generatedWords * 4;
        for (int m = 0; m < 4; ++m) {
            roundKeys[storeIdx + m] = temp[m];
        }

        generatedWords = generatedWords + 1;
    }

    /* --------------------------------------------------------------------
       4.  Helper lambdas for the four AES transformations
       -------------------------------------------------------------------- */
    auto addRoundKey = [&](int *state, int round) {
        for (int idx = 0; idx < 16; ++idx) {
            state[idx] = state[idx] ^ roundKeys[round * 16 + idx];
        }
    };

    auto subBytes = [&](int *state) {
        for (int idx = 0; idx < 16; ++idx) {
            state[idx] = sbox[state[idx]];
        }
    };

    auto shiftRows = [&](int *state) {
        // Row 1 (index 1,5,9,13) shift left by 1
        int tmp = state[1];
        state[1]  = state[5];
        state[5]  = state[9];
        state[9]  = state[13];
        state[13] = tmp;

        // Row 2 (index 2,6,10,14) shift left by 2
        int tmp1 = state[2];
        int tmp2 = state[6];
        state[2]  = state[10];
        state[6]  = state[14];
        state[10] = tmp1;
        state[14] = tmp2;

        // Row 3 (index 3,7,11,15) shift left by 3 (right by 1)
        tmp = state[15];
        state[15] = state[11];
        state[11] = state[7];
        state[7]  = state[3];
        state[3]  = tmp;
    };

    auto mixColumns = [&](int *state) {
        int col = 0;
        while (col < 4) {
            int a0 = state[col*4 + 0];
            int a1 = state[col*4 + 1];
            int a2 = state[col*4 + 2];
            int a3 = state[col*4 + 3];

            int xtime0 = (a0 << 1) ^ ((a0 & 0x80) ? 0x11b : 0);
            int xtime1 = (a1 << 1) ^ ((a1 & 0x80) ? 0x11b : 0);
            int xtime2 = (a2 << 1) ^ ((a2 & 0x80) ? 0x11b : 0);
            int xtime3 = (a3 << 1) ^ ((a3 & 0x80) ? 0x11b : 0);

            state[col*4 + 0] = (xtime0 ^ a1 ^ xtime1 ^ a1 ^ a2 ^ a3) & 0xFF;
            state[col*4 + 1] = (a0 ^ xtime1 ^ a2 ^ xtime2 ^ a2 ^ a3) & 0xFF;
            state[col*4 + 2] = (a0 ^ a1 ^ xtime2 ^ a3 ^ xtime3 ^ a3) & 0xFF;
            state[col*4 + 3] = (xtime0 ^ a0 ^ a1 ^ a2 ^ xtime3 ^ a3) & 0xFF;

            col = col + 1;
        }
    };

    /* --------------------------------------------------------------------
       5.  Perform AES‑128 encryption (ECB – single block)
       -------------------------------------------------------------------- */
    int *state = new int[16];
    for (int i = 0; i < 16; ++i) {
        state[i] = plainBlock[i];
    }

    // Initial round
    addRoundKey(state, 0);

    // 9 main rounds
    for (int round = 1; round <= 9; ++round) {
        subBytes(state);
        shiftRows(state);
        mixColumns(state);
        addRoundKey(state, round);
    }

    // Final round (no MixColumns)
    subBytes(state);
    shiftRows(state);
    addRoundKey(state, 10);

    /* --------------------------------------------------------------------
       6.  Output the ciphertext in hex format
       -------------------------------------------------------------------- */
    std::cout << "Ciphertext: ";
    for (int i = 0; i < 16; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (state[i] & 0xFF);
        if (i != 15) std::cout << " ";
    }
    std::cout << std::dec << std::endl;

    /* --------------------------------------------------------------------
       7.  Clean up heap memory
       -------------------------------------------------------------------- */
    delete[] secretKey;
    delete[] plainBlock;
    delete[] roundKeys;
    delete[] state;

    return 0;
}
