#include <iostream>
#include <iomanip>

/* LLM input variant 6: ordered-structured */

int main() {
    /* ------------------------------------------------------------------ *
     *  AES‑128 implementation – version #9                               *
     *  All code lives inside main(), uses only int and float types.      *
     * ------------------------------------------------------------------ */

    /* ---- 1.  Fixed data (S‑box, Rcon) --------------------------------- */
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

    int rcon[11] = {0x00,0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36};

    /* ---- 2.  Input data (plaintext & key) ------------------------------ */
    int pt[16] = {
        0x00,0x01,0x02,0x03,
        0x04,0x05,0x06,0x07,
        0x08,0x09,0x0A,0x0B,
        0x0C,0x0D,0x0E,0x0F
    };

    int key[16] = {
        0x0F,0x0E,0x0D,0x0C,
        0x0B,0x0A,0x09,0x08,
        0x07,0x06,0x05,0x04,
        0x03,0x02,0x01,0x00
    };

    /* ---- 3.  Helper lambdas ------------------------------------------- */
    auto xtime = [](int x)->int{
        int r = (x << 1) & 0xFF;
        if (x & 0x80) r ^= 0x1b;
        return r;
    };

    auto subWord = [&](int *w){
        for (int i = 0; i < 4; ++i) w[i] = sbox[w[i]];
    };

    auto rotWord = [&](int *w){
        int t = w[0];
        w[0] = w[1];
        w[1] = w[2];
        w[2] = w[3];
        w[3] = t;
    };

    /* ---- 4.  Key expansion (44 words) --------------------------------- */
    int expKey[44][4];
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            expKey[i][j] = key[4*i + j];
        }
    }

    for (int i = 4; i < 44; ++i) {
        int temp[4];
        for (int j = 0; j < 4; ++j) temp[j] = expKey[i-1][j];

        if (i % 4 == 0) {
            rotWord(temp);
            subWord(temp);
            temp[0] ^= rcon[i/4];
        }

        for (int j = 0; j < 4; ++j) {
            expKey[i][j] = expKey[i-4][j] ^ temp[j];
        }
    }

    /* ---- 5.  State matrix (column‑major) ------------------------------ */
    int state[4][4];
    for (int c = 0; c < 4; ++c)
        for (int r = 0; r < 4; ++r)
            state[r][c] = pt[c*4 + r];

    /* ---- 6.  AddRoundKey (initial) ----------------------------------- */
    for (int c = 0; c < 4; ++c)
        for (int r = 0; r < 4; ++r)
            state[r][c] ^= expKey[c][r];

    /* ---- 7.  Main rounds (9 times) ------------------------------------ */
    for (int round = 1; round <= 9; ++round) {
        /* SubBytes */
        for (int c = 0; c < 4; ++c)
            for (int r = 0; r < 4; ++r)
                state[r][c] = sbox[state[r][c]];

        /* ShiftRows (different loop order) */
        for (int r = 1; r < 4; ++r) {
            int tmp[4];
            for (int c = 0; c < 4; ++c)
                tmp[c] = state[r][(c + r) & 3];
            for (int c = 0; c < 4; ++c)
                state[r][c] = tmp[c];
        }

        /* MixColumns */
        for (int c = 0; c < 4; ++c) {
            int a0 = state[0][c];
            int a1 = state[1][c];
            int a2 = state[2][c];
            int a3 = state[3][c];

            int t0 = xtime(a0) ^ xtime(a1) ^ a1 ^ a2 ^ a3;
            int t1 = a0 ^ xtime(a1) ^ xtime(a2) ^ a2 ^ a3;
            int t2 = a0 ^ a1 ^ xtime(a2) ^ xtime(a3) ^ a3;
            int t3 = xtime(a0) ^ a0 ^ a1 ^ a2 ^ xtime(a3);

            state[0][c] = t0 & 0xFF;
            state[1][c] = t1 & 0xFF;
            state[2][c] = t2 & 0xFF;
            state[3][c] = t3 & 0xFF;
        }

        /* AddRoundKey */
        for (int c = 0; c < 4; ++c)
            for (int r = 0; r < 4; ++r)
                state[r][c] ^= expKey[round*4 + c][r];
    }

    /* ---- 8.  Final round (no MixColumns) ----------------------------- */
    /* SubBytes */
    for (int c = 0; c < 4; ++c)
        for (int r = 0; r < 4; ++r)
            state[r][c] = sbox[state[r][c]];

    /* ShiftRows */
    for (int r = 1; r < 4; ++r) {
        int tmp[4];
        for (int c = 0; c < 4; ++c)
            tmp[c] = state[r][(c + r) & 3];
        for (int c = 0; c < 4; ++c)
            state[r][c] = tmp[c];
    }

    /* AddRoundKey */
    for (int c = 0; c < 4; ++c)
        for (int r = 0; r < 4; ++r)
            state[r][c] ^= expKey[40 + c][r];

    /* ---- 9.  Output ciphertext --------------------------------------- */
    int ct[16];
    for (int c = 0; c < 4; ++c)
        for (int r = 0; r < 4; ++r)
            ct[c*4 + r] = state[r][c];

    std::cout << "Ciphertext: ";
    for (int i = 0; i < 16; ++i) {
        std::cout << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
                  << (ct[i] & 0xFF);
        if (i != 15) std::cout << " ";
    }
    std::cout << std::dec << std::endl;

    return 0;
}
