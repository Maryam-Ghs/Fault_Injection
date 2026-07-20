#include <cstdio>
#include <cstring>

/* LLM input variant 8: sparse-skewed */
int main()
{
    /* ----------  S‑box and multiplication tables (int, no const) ---------- */
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
    int mul2[256]; int mul3[256];
    for (int i = 0; i < 256; ++i) {
        int x = i << 1; mul2[i] = ((x ^ 0x1B) & 0xFF) * (x >> 8);
        mul3[i] = (mul2[i] ^ i) & 0xFF;
    }

    /* -----------------------  Key schedule  ----------------------- */
    int key[32];                     /* 256‑bit key, generated below      */
    for (int i = 0; i < 32; ++i) key[i] = (i == 0 || i == 31) ? 0xAA : 0;   /* sparse deterministic test key */

    int roundKey[15][16];            /* 15 round keys (initial + 14)      */

    /* copy original key as first two round keys */
    for (int i = 0; i < 16; ++i) roundKey[0][i] = key[i];
    for (int i = 0; i < 16; ++i) roundKey[1][i] = key[16 + i];

    int rcon[15] = {1,2,4,8,16,32,64,128,27,54,108,216,171,77,154};

    for (int i = 2; i < 15; ++i) {
        int temp[4];
        /* ----  generate temp from previous word  ---- */
        for (int j = 0; j < 4; ++j) temp[j] = roundKey[i - 1][12 + j];
        /* ----  RotWord  ---- */
        int t0 = temp[1]; temp[1] = temp[2]; temp[2] = temp[3]; temp[3] = t0;
        /* ----  SubWord  ---- */
        for (int j = 0; j < 4; ++j) temp[j] = sbox[temp[j]];
        /* ----  Rcon  ---- */
        temp[0] ^= rcon[i - 2];
        /* ----  first half of round key  ---- */
        for (int j = 0; j < 4; ++j) {
            roundKey[i][j] = (roundKey[i - 2][j] ^ temp[j]) & 0xFF;
        }
        /* ----  second half of round key  ---- */
        for (int j = 4; j < 8; ++j) {
            roundKey[i][j] = (roundKey[i - 2][j] ^ roundKey[i][j - 4]) & 0xFF;
        }
        for (int j = 8; j < 12; ++j) {
            roundKey[i][j] = (roundKey[i - 2][j] ^ roundKey[i][j - 4]) & 0xFF;
        }
        for (int j = 12; j < 16; ++j) {
            roundKey[i][j] = (roundKey[i - 2][j] ^ roundKey[i][j - 4]) & 0xFF;
        }
    }

    /* --------------------  AES encrypt single block  -------------------- */
    auto aesEncrypt = [&](int in[16], int out[16]) {
        int state[16];
        for (int i = 0; i < 16; ++i) state[i] = (in[i] ^ roundKey[0][i]) & 0xFF;

        for (int round = 1; round < 14; ++round) {
            /* SubBytes */
            for (int i = 0; i < 16; ++i) state[i] = sbox[state[i]];
            /* ShiftRows (manual unroll) */
            int tmp;
            /* row 1 */
            tmp = state[1]; state[1] = state[5]; state[5] = state[9];
            state[9] = state[13]; state[13] = tmp;
            /* row 2 */
            tmp = state[2]; state[2] = state[10]; state[10] = tmp;
            tmp = state[6]; state[6] = state[14]; state[14] = tmp;
            /* row 3 */
            tmp = state[3]; state[3] = state[15]; state[15] = state[11];
            state[11] = state[7]; state[7] = tmp;
            /* MixColumns (manual unroll) */
            for (int c = 0; c < 4; ++c) {
                int i0 = c*4, i1 = i0+1, i2 = i0+2, i3 = i0+3;
                int a0 = state[i0], a1 = state[i1], a2 = state[i2], a3 = state[i3];
                int r0 = (mul2[a0] ^ mul3[a1] ^ a2 ^ a3) & 0xFF;
                int r1 = (a0 ^ mul2[a1] ^ mul3[a2] ^ a3) & 0xFF;
                int r2 = (a0 ^ a1 ^ mul2[a2] ^ mul3[a3]) & 0xFF;
                int r3 = (mul3[a0] ^ a1 ^ a2 ^ mul2[a3]) & 0xFF;
                state[i0]=r0; state[i1]=r1; state[i2]=r2; state[i3]=r3;
            }
            /* AddRoundKey */
            for (int i = 0; i < 16; ++i) state[i] ^= roundKey[round][i];
        }

        /* ----- final round (no MixColumns) ----- */
        for (int i = 0; i < 16; ++i) state[i] = sbox[state[i]];
        /* ShiftRows final */
        int tmp;
        tmp = state[1]; state[1] = state[5]; state[5] = state[9];
        state[9] = state[13]; state[13] = tmp;
        tmp = state[2]; state[2] = state[10]; state[10] = tmp;
        tmp = state[6]; state[6] = state[14]; state[14] = tmp;
        tmp = state[3]; state[3] = state[15]; state[15] = state[11];
        state[11] = state[7]; state[7] = tmp;
        /* AddRoundKey final */
        for (int i = 0; i < 16; ++i) out[i] = (state[i] ^ roundKey[14][i]) & 0xFF;
    };

    /* ----------------------  CMAC subkey generation  ---------------------- */
    int L[16]; int zeroBlock[16] = {0};
    aesEncrypt(zeroBlock, L);

    auto leftShiftOne = [&](int in[16], int out[16]) {
        int carry = 0;
        for (int i = 15; i >= 0; --i) {
            int val = (in[i] << 1) | carry;
            out[i] = val & 0xFF;
            carry = (in[i] & 0x80) ? 1 : 0;
        }
    };
    int K1[16]; int K2[16];
    leftShiftOne(L, K1);
    if (L[0] & 0x80) K1[15] ^= 0x87;
    leftShiftOne(K1, K2);
    if (K1[0] & 0x80) K2[15] ^= 0x87;

    /* ---------------------------  Test vectors  --------------------------- */
    int messages[][33] = {
        {},                                 /* length 0  */
        {0xFF},                             /* length 1  */
        {0x01,0,0,0,0,0,0,0,0,0,0,0,0,0,0},            /* 15 bytes sparse */
        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0x01},          /* 16 bytes sparse */
        {0,0,0,0,0,0,0,0,0x01,0,0,0,0,0,0,0,0},/* 17 bytes sparse */
        {0x01,0,0,0,0,0,0,0,0,0,0x02,0,0,0,0,0,0,0,0,0,0x03,0,0,0,0,0,0,0,0,0,0,0,0x04}/* 33 bytes sparse */
    };
    int msgLen[] = {0,1,15,16,17,33};
    int testCount = 6;

    /* ---------------------------  CMAC compute  --------------------------- */
    for (int t = 0; t < testCount; ++t) {
        int mlen = msgLen[t];
        int blocks = (mlen + 15) / 16;                /* number of 16‑byte blocks */
        int lastComplete = (mlen % 16 == 0) && (mlen != 0);
        int M[4][16];                                 /* up to 4 blocks, stack */
        for (int b = 0; b < 4; ++b) for (int i = 0; i < 16; ++i) M[b][i] = 0;

        /* copy message into M, pad later */
        for (int i = 0; i < mlen; ++i) {
            int b = i / 16; int off = i % 16;
            M[b][off] = messages[t][i];
        }

        /* ---------- padding and subkey selection ---------- */
        int X[16]; for (int i = 0; i < 16; ++i) X[i] = 0;
        if (lastComplete) {
            /* last block is complete → XOR with K1 */
            for (int i = 0; i < 16; ++i) M[blocks-1][i] ^= K1[i];
        } else {
            /* incomplete block → pad 0x80 then zeros, XOR with K2 */
            int padIndex = mlen % 16;
            M[blocks-1][padIndex] ^= 0x80;
            for (int i = 0; i < 16; ++i) M[blocks-1][i] ^= K2[i];
        }

        /* ---------- process blocks (manual unroll, up to 4) ---------- */
        if (blocks >= 1) {
            int tmp[16];
            for (int i = 0; i < 16; ++i) tmp[i] = (X[i] ^ M[0][i]) & 0xFF;
            aesEncrypt(tmp, X);
        }
        if (blocks >= 2) {
            int tmp[16];
            for (int i = 0; i < 16; ++i) tmp[i] = (X[i] ^ M[1][i]) & 0xFF;
            aesEncrypt(tmp, X);
        }
        if (blocks >= 3) {
            int tmp[16];
            for (int i = 0; i < 16; ++i) tmp[i] = (X[i] ^ M[2][i]) & 0xFF;
            aesEncrypt(tmp, X);
        }
        if (blocks >= 4) {
            int tmp[16];
            for (int i = 0; i < 16; ++i) tmp[i] = (X[i] ^ M[3][i]) & 0xFF;
            aesEncrypt(tmp, X);
        }

        /* ---------- output ---------- */
        std::printf("Test %d (len=%d): ", t+1, mlen);
        for (int i = 0; i < 16; ++i) std::printf("%02x", X[i]);
        std::printf("\n");
    }
    return 0;
}
