#include <iostream>
#include <iomanip>

/* LLM input variant 7: reverse-adversarial */
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
       2.  Prepare deterministic test vectors (key & plaintext) – reversed order
       -------------------------------------------------------------------- */
    int *secretKey   = new int[16];
    int *plainBlock  = new int[16];
    int i = 0;
    while (i < 16) {
        secretKey[i]   = 15 - i;                     // 0x0F,0x0E,...,0x00
        plainBlock[i]  = ((15 - i) * 0x11) & 0xFF;   // 0xF0,0xDF,...,0x00
        i = i + 1;
    }

    /* --------------------------------------------------------------------
       3.  Expand the key (44 words -> 11 round keys, each 16 bytes)
       -------------------------------------------------------------------- */
    int *roundKeys = new int[11 * 16];   // roundKeys[round*16 + byte]

    // copy first round key (the original key)
    i = 0;
    while (i < 16) {
        roundKeys[i] = secretKey[i];
        i = i + 1;
    }

    // temporary word for key schedule (4 bytes)
    int temp[4];
    int generatedWords = 4; // we already have 4 words (16 bytes)

    while (generatedWords < 44) {
        // copy previous word
        int srcIdx = (generatedWords - 1) * 4;
        int dstIdx = 0;
        while (dstIdx < 4) {
            temp[dstIdx] = roundKeys[srcIdx + dstIdx];
            dstIdx = dstIdx + 1;
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
            int j = 0;
            while (j < 4) {
                temp[j] = sbox[temp[j]];
                j = j + 1;
            }

            // Rcon XOR on first byte
            temp[0] = temp[0] ^ rcon[(generatedWords/4)-1];
        }

        // XOR with word 4 positions earlier
        int prevIdx = (generatedWords - 4) * 4;
        int k = 0;
        while (k < 4) {
            temp[k] = temp[k] ^ roundKeys[prevIdx + k];
            k = k + 1;
        }

        // store the new word
        int storeIdx = generatedWords * 4;
        int m = 0;
        while (m < 4) {
            roundKeys[storeIdx + m] = temp[m];
            m = m + 1;
        }

        generatedWords = generatedWords + 1;
    }

    /* --------------------------------------------------------------------
       4.  Helper lambdas for the four AES transformations
       -------------------------------------------------------------------- */
    auto addRoundKey = [&](int *state, int round) {
        int idx = 0;
        while (idx < 16) {
            state[idx] = state[idx] ^ roundKeys[round * 16 + idx];
            idx = idx + 1;
        }
    };

    auto subBytes = [&](int *state) {
        int idx = 0;
        while (idx < 16) {
            state[idx] = sbox[state[idx]];
            idx = idx + 1;
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

            // GF(2^8) multiplication helpers (using int arithmetic only)
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
    // copy plaintext into state
    i = 0;
    while (i < 16) {
        state[i] = plainBlock[i];
        i = i + 1;
    }

    // Initial round
    addRoundKey(state, 0);

    // 9 main rounds
    int round = 1;
    while (round <= 9) {
        subBytes(state);
        shiftRows(state);
        mixColumns(state);
        addRoundKey(state, round);
        round = round + 1;
    }

    // Final round (no MixColumns)
    subBytes(state);
    shiftRows(state);
    addRoundKey(state, 10);

    /* --------------------------------------------------------------------
       6.  Output the ciphertext in hex format
       -------------------------------------------------------------------- */
    std::cout << "Ciphertext: ";
    i = 0;
    while (i < 16) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (state[i] & 0xFF);
        if (i != 15) std::cout << " ";
        i = i + 1;
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
