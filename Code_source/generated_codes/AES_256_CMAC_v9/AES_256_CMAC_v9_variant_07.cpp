#include <cstdio>
#include <cstring>

/* LLM input variant 7: reverse-adversarial */

int main()
{
    /* ----------  S‑box and multiplication tables (int, no const) ---------- */
    int sbox[256] = {
        22,187,84,176,15,45,153,65,104,66,230,191,13,137,161,140,
        223,40,85,206,233,135,30,155,148,142,217,105,17,152,248,225,
        158,29,193,134,185,87,53,97,14,246,3,72,102,181,62,112,
        138,139,189,75,31,116,221,232,198,180,166,28,46,37,120,186,
        8,174,122,101,234,244,86,108,169,78,213,141,109,55,200,231,
        121,228,149,145,98,172,211,194,92,36,6,73,10,58,50,224,
        219,11,94,222,20,184,238,70,136,144,42,34,220,79,129,96,
        115,25,93,100,61,126,167,196,23,68,151,95,236,19,12,205,
        210,243,255,16,33,218,182,188,245,56,157,146,143,64,163,81,
        168,159,60,80,127,2,249,69,133,51,77,67,251,170,239,208,
        207,88,76,74,57,190,203,106,91,177,252,32,237,0,209,83,
        132,47,227,41,179,214,59,82,160,90,110,27,26,44,131,9,
        117,178,39,235,226,128,18,7,154,5,150,24,195,35,199,4,
        21,49,216,113,241,229,165,52,204,247,63,54,38,147,253,183,
        192,114,164,156,175,162,212,173,240,71,89,250,125,201,130,202,
        118,171,215,254,43,103,1,48,197,111,107,242,123,119,124,99
    };
    int mul2[256]; int mul3[256];
    for (int i = 0; i < 256; ++i) {
        int x = i << 1; mul2[i] = ((x ^ 0x1B) & 0xFF) * (x >> 8);
        mul3[i] = (mul2[i] ^ i) & 0xFF;
    }

    /* -----------------------  Key schedule  ----------------------- */
    int key[32];                     /* 256‑bit key, generated below      */
    for (int i = 0; i < 32; ++i) key[i] = 31 - i;   /* reversed deterministic test key */

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
        {0x20,0x1f,0x1e,0x1d,0x1c,0x1b,0x1a,0x19,0x18,0x17,0x16,0x15,0x14,0x13,0x12,0x11,0x10,0x0f,0x0e,0x0d,0x0c,0x0b,0x0a,0x09,0x08,0x07,0x06,0x05,0x04,0x03,0x02,0x01,0x00},
        {0x01,0xff,0xee,0xdd,0xcc,0xbb,0xaa,0x99,0x88,0x77,0x66,0x55,0x44,0x33,0x22,0x11,0x00},
        {0xff,0xee,0xdd,0xcc,0xbb,0xaa,0x99,0x88,0x77,0x66,0x55,0x44,0x33,0x22,0x11,0x00},
        {0xee,0xdd,0xcc,0xbb,0xaa,0x99,0x88,0x77,0x66,0x55,0x44,0x33,0x22,0x11,0x00},
        {0x01},
        {}
    };
    int msgLen[] = {33,17,16,15,1,0};
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
