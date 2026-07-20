#include <cstdio>
#include <cstdlib>

/* LLM input variant 2: small-diverse */
int main()
{
    /*--------------------------------------------------------------
     *  Seed and helper lambdas (only int / float types are used)
     *--------------------------------------------------------------*/
    std::srand(123456);                         // deterministic “random”

    auto mul2 = [](int x) -> int               // GF(2^8) ×2
    {
        int r = x << 1;
        if (x & 0x80) r ^= 0x1b;
        return r & 0xFF;
    };
    auto mul3 = [&](int x) -> int               // GF(2^8) ×3 = ×2 ⊕ x
    {
        return (mul2(x) ^ x) & 0xFF;
    };

    /*--------------------------------------------------------------
     *  S‑box and Rcon (stack allocated, mutable but never changed)
     *--------------------------------------------------------------*/
    int sboxArr[256] = {
        0x63,0x7c,0x77,0x7b,0xf2,0x6b,0x6f,0xc5,0x30,0x01,0x67,0x2b,0xfe,0xd7,0xab,0x76,
        0xca,0x82,0xc9,0x7d,0xfa,0x59,0x47,0xf0,0xad,0xd4,0xa2,0xaf,0x9c,0xa4,0x72,0xc0,
        0xb7,0xfd,0x93,0x26,0x36,0x3f,0xf7,0xcc,0x34,0xa5,0xe5,0xf1,0x71,0xd8,0x31,0x15,
        0x04,0xc7,0x23,0xc3,0x18,0x96,0x05,0x9a,0x07,0x12,0x80,0xe2,0xeb,0x27,0xb2,0x75,
        0x09,0x83,0x2c,0x1a,0x1b,0x6e,0x5a,0xa0,0x52,0x3b,0xd6,0xb3,0x29,0xe3,0x2f,0x84,
        0x53,0xd1,0x00,0xed,0x20,0xfc,0xb1,0x5b,0x6a,0xcb,0xbe,0x39,0x4a,0x4c,0x58,0xcf,
        0xd0,0xef,0xaa,0xfb,0x43,0x4d,0x33,0x85,0x45,0xf9,0x02,0x7f,0x50,0x3c,0x9f,0xa8,
        0x51,0xa3,0x40,0x8f,0x92,0x9d,0x38,0xf5,0xbc,0xb6,0xda,0x21,0x10,0xff,0xf3,0xd2,
        0xcd,0x0c,0x13,0xec,0x5f,0x97,0x44,0x17,0xc4,0xa7,0x7e,0x3d,0x64,0x5d,0x19,0x73,
        0x60,0x81,0x4f,0xdc,0x22,0x2a,0x90,0x88,0x46,0xee,0xb8,0x14,0xde,0x5e,0x0b,0xdb,
        0xe0,0x32,0x3a,0x0a,0x49,0x06,0x24,0x5c,0xc2,0xd3,0xac,0x62,0x91,0x95,0xe4,0x79,
        0xe7,0xc8,0x37,0x6d,0x8d,0xd5,0x4e,0xa9,0x6c,0x56,0xf4,0xea,0x65,0x7a,0xae,0x08,
        0xba,0x78,0x25,0x2e,0x1c,0xa6,0xb4,0xc6,0xe8,0xdd,0x74,0x1f,0x4b,0xbd,0x8b,0x8a,
        0x70,0x3e,0xb5,0x66,0x48,0x03,0xf6,0x0e,0x61,0x35,0x57,0xb9,0x86,0xc1,0x1d,0x9e,
        0xe1,0xf8,0x98,0x11,0x69,0xd9,0x8e,0x94,0x9b,0x1e,0x87,0xe9,0xce,0x55,0x28,0xdf,
        0x8c,0xa1,0x89,0x0d,0xbf,0xe6,0x42,0x68,0x41,0x99,0x2d,0x0f,0xb0,0x54,0xbb,0x16
    };

    int rconArr[10] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36};

    /*--------------------------------------------------------------
     *  Deterministic small key, IV and plaintext
     *--------------------------------------------------------------*/
    int masterKey[16] = {
        0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,
        0x88,0x99,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF
    };

    int initVec[16] = {
        0xFF,0xEE,0xDD,0xCC,0xBB,0xAA,0x99,0x88,
        0x77,0x66,0x55,0x44,0x33,0x22,0x11,0x00
    };

    int ptLen = 16;                       // single AES block
    int plainData[16] = {
        0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
        0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F
    };

    /*--------------------------------------------------------------
     *  Key expansion (AES‑128 => 11 round keys, 176 bytes)
     *--------------------------------------------------------------*/
    int roundKeys[11][16];
    int idx = 0;
    while (idx < 16) { roundKeys[0][idx] = masterKey[idx]; ++idx; }

    int expPos = 16;
    int tempWord[4];
    while (expPos < 176)
    {
        /* copy previous word */
        int wIdx = expPos - 4;
        tempWord[0] = roundKeys[wIdx/16][wIdx%16];
        tempWord[1] = roundKeys[wIdx/16][(wIdx%16)+1];
        tempWord[2] = roundKeys[wIdx/16][(wIdx%16)+2];
        tempWord[3] = roundKeys[wIdx/16][(wIdx%16)+3];

        if (expPos % 16 == 0)
        {
            /* RotWord */
            int t = tempWord[0];
            tempWord[0] = tempWord[1];
            tempWord[1] = tempWord[2];
            tempWord[2] = tempWord[3];
            tempWord[3] = t;

            /* SubWord */
            tempWord[0] = sboxArr[tempWord[0]];
            tempWord[1] = sboxArr[tempWord[1]];
            tempWord[2] = sboxArr[tempWord[2]];
            tempWord[3] = sboxArr[tempWord[3]];

            /* Rcon */
            tempWord[0] = (tempWord[0] ^ rconArr[(expPos/16)-1]) & 0xFF;
        }

        /* XOR with word 16 bytes earlier */
        int prevPos = expPos - 16;
        roundKeys[expPos/16][(expPos%16)    ] = (roundKeys[prevPos/16][prevPos%16] ^ tempWord[0]) & 0xFF;
        roundKeys[expPos/16][(expPos%16) + 1] = (roundKeys[prevPos/16][(prevPos%16)+1] ^ tempWord[1]) & 0xFF;
        roundKeys[expPos/16][(expPos%16) + 2] = (roundKeys[prevPos/16][(prevPos%16)+2] ^ tempWord[2]) & 0xFF;
        roundKeys[expPos/16][(expPos%16) + 3] = (roundKeys[prevPos/16][(prevPos%16)+3] ^ tempWord[3]) & 0xFF;

        expPos += 4;
    }

    /*--------------------------------------------------------------
     *  Inline AES‑128 encryption of a single block (manual unroll)
     *--------------------------------------------------------------*/
    auto encryptBlock = [&](int in[16], int out[16])
    {
        int state[16];
        /* copy input */
        state[0] = in[0];  state[1] = in[1];  state[2] = in[2];  state[3] = in[3];
        state[4] = in[4];  state[5] = in[5];  state[6] = in[6];  state[7] = in[7];
        state[8] = in[8];  state[9] = in[9];  state[10] = in[10]; state[11] = in[11];
        state[12] = in[12];state[13] = in[13];state[14] = in[14];state[15] = in[15];

        /* ---------- AddRoundKey (round 0) ---------- */
        state[0]  ^= roundKeys[0][0];  state[1]  ^= roundKeys[0][1];
        state[2]  ^= roundKeys[0][2];  state[3]  ^= roundKeys[0][3];
        state[4]  ^= roundKeys[0][4];  state[5]  ^= roundKeys[0][5];
        state[6]  ^= roundKeys[0][6];  state[7]  ^= roundKeys[0][7];
        state[8]  ^= roundKeys[0][8];  state[9]  ^= roundKeys[0][9];
        state[10] ^= roundKeys[0][10]; state[11] ^= roundKeys[0][11];
        state[12] ^= roundKeys[0][12]; state[13] ^= roundKeys[0][13];
        state[14] ^= roundKeys[0][14]; state[15] ^= roundKeys[0][15];

        /* ---------- Rounds 1 … 9 ---------- */
        int round = 1;
        while (round <= 9)
        {
            /* SubBytes */
            state[0]  = sboxArr[state[0]];  state[1]  = sboxArr[state[1]];
            state[2]  = sboxArr[state[2]];  state[3]  = sboxArr[state[3]];
            state[4]  = sboxArr[state[4]];  state[5]  = sboxArr[state[5]];
            state[6]  = sboxArr[state[6]];  state[7]  = sboxArr[state[7]];
            state[8]  = sboxArr[state[8]];  state[9]  = sboxArr[state[9]];
            state[10] = sboxArr[state[10]]; state[11] = sboxArr[state[11]];
            state[12] = sboxArr[state[12]]; state[13] = sboxArr[state[13]];
            state[14] = sboxArr[state[14]]; state[15] = sboxArr[state[15]];

            /* ShiftRows (manual mapping) */
            int tmp1 = state[1];  int tmp5 = state[5];  int tmp9 = state[9];  int tmp13 = state[13];
            state[1]  = state[5];   state[5]  = state[9];
            state[9]  = state[13];  state[13] = tmp1;

            int tmp2 = state[2];   int tmp6 = state[6];
            int tmp10 = state[10]; int tmp14 = state[14];
            state[2]  = state[10];  state[6]  = state[14];
            state[10] = tmp2;       state[14] = tmp6;

            int tmp3 = state[3];   int tmp7 = state[7];
            int tmp11 = state[11]; int tmp15 = state[15];
            state[3]  = state[15];  state[7]  = tmp3;
            state[11] = tmp7;       state[15] = tmp11;

            /* MixColumns (expanded multi‑step) */
            int c0 = state[0]; int c1 = state[1]; int c2 = state[2]; int c3 = state[3];
            int m0 = mul2(c0) ^ mul3(c1) ^ c2 ^ c3;
            int m1 = c0 ^ mul2(c1) ^ mul3(c2) ^ c3;
            int m2 = c0 ^ c1 ^ mul2(c2) ^ mul3(c3);
            int m3 = mul3(c0) ^ c1 ^ c2 ^ mul2(c3);
            state[0] = m0; state[1] = m1; state[2] = m2; state[3] = m3;

            c0 = state[4]; c1 = state[5]; c2 = state[6]; c3 = state[7];
            m0 = mul2(c0) ^ mul3(c1) ^ c2 ^ c3;
            m1 = c0 ^ mul2(c1) ^ mul3(c2) ^ c3;
            m2 = c0 ^ c1 ^ mul2(c2) ^ mul3(c3);
            m3 = mul3(c0) ^ c1 ^ c2 ^ mul2(c3);
            state[4] = m0; state[5] = m1; state[6] = m2; state[7] = m3;

            c0 = state[8]; c1 = state[9]; c2 = state[10]; c3 = state[11];
            m0 = mul2(c0) ^ mul3(c1) ^ c2 ^ c3;
            m1 = c0 ^ mul2(c1) ^ mul3(c2) ^ c3;
            m2 = c0 ^ c1 ^ mul2(c2) ^ mul3(c3);
            m3 = mul3(c0) ^ c1 ^ c2 ^ mul2(c3);
            state[8] = m0; state[9] = m1; state[10] = m2; state[11] = m3;

            c0 = state[12]; c1 = state[13]; c2 = state[14]; c3 = state[15];
            m0 = mul2(c0) ^ mul3(c1) ^ c2 ^ c3;
            m1 = c0 ^ mul2(c1) ^ mul3(c2) ^ c3;
            m2 = c0 ^ c1 ^ mul2(c2) ^ mul3(c3);
            m3 = mul3(c0) ^ c1 ^ c2 ^ mul2(c3);
            state[12] = m0; state[13] = m1; state[14] = m2; state[15] = m3;

            /* AddRoundKey */
            int *rk = roundKeys[round];
            state[0]  ^= rk[0];  state[1]  ^= rk[1];
            state[2]  ^= rk[2];  state[3]  ^= rk[3];
            state[4]  ^= rk[4];  state[5]  ^= rk[5];
            state[6]  ^= rk[6];  state[7]  ^= rk[7];
            state[8]  ^= rk[8];  state[9]  ^= rk[9];
            state[10] ^= rk[10]; state[11] ^= rk[11];
            state[12] ^= rk[12]; state[13] ^= rk[13];
            state[14] ^= rk[14]; state[15] ^= rk[15];

            ++round;
        }

        /* ---------- Final round (no MixColumns) ---------- */
        /* SubBytes */
        state[0]  = sboxArr[state[0]];  state[1]  = sboxArr[state[1]];
        state[2]  = sboxArr[state[2]];  state[3]  = sboxArr[state[3]];
        state[4]  = sboxArr[state[4]];  state[5]  = sboxArr[state[5]];
        state[6]  = sboxArr[state[6]];  state[7]  = sboxArr[state[7]];
        state[8]  = sboxArr[state[8]];  state[9]  = sboxArr[state[9]];
        state[10] = sboxArr[state[10]]; state[11] = sboxArr[state[11]];
        state[12] = sboxArr[state[12]]; state[13] = sboxArr[state[13]];
        state[14] = sboxArr[state[14]]; state[15] = sboxArr[state[15]];

        /* ShiftRows */
        int a1 = state[1]; int a5 = state[5]; int a9 = state[9]; int a13 = state[13];
        state[1] = a5; state[5] = a9; state[9] = a13; state[13] = a1;

        int a2 = state[2]; int a6 = state[6];
        int a10 = state[10]; int a14 = state[14];
        state[2] = a10; state[6] = a14; state[10] = a2; state[14] = a6;

        int a3 = state[3]; int a7 = state[7];
        int a11 = state[11]; int a15 = state[15];
        state[3] = a15; state[7] = a3; state[11] = a7; state[15] = a11;

        /* AddRoundKey (round 10) */
        int *rk10 = roundKeys[10];
        state[0]  ^= rk10[0];  state[1]  ^= rk10[1];
        state[2]  ^= rk10[2];  state[3]  ^= rk10[3];
        state[4]  ^= rk10[4];  state[5]  ^= rk10[5];
        state[6]  ^= rk10[6];  state[7]  ^= rk10[7];
        state[8]  ^= rk10[8];  state[9]  ^= rk10[9];
        state[10] ^= rk10[10]; state[11] ^= rk10[11];
        state[12] ^= rk10[12]; state[13] ^= rk10[13];
        state[14] ^= rk10[14]; state[15] ^= rk10[15];

        /* copy to output */
        out[0]  = state[0];  out[1]  = state[1];
        out[2]  = state[2];  out[3]  = state[3];
        out[4]  = state[4];  out[5]  = state[5];
        out[6]  = state[6];  out[7]  = state[7];
        out[8]  = state[8];  out[9]  = state[9];
        out[10] = state[10]; out[11] = state[11];
        out[12] = state[12]; out[13] = state[13];
        out[14] = state[14]; out[15] = state[15];
    };

    /*--------------------------------------------------------------
     *  CFB‑128 encryption (feedback = ciphertext)
     *--------------------------------------------------------------*/
    int cipherData[16];
    int feedbackBlock[16];
    for (int i = 0; i < 16; ++i) feedbackBlock[i] = initVec[i];

    int processed = 0;
    while (processed < ptLen)
    {
        int keystream[16];
        encryptBlock(feedbackBlock, keystream);

        /* XOR with current plaintext block */
        int j = 0;
        while (j < 16)
        {
            cipherData[processed + j] = (keystream[j] ^ plainData[processed + j]) & 0xFF;
            ++j;
        }

        /* feedback becomes ciphertext block */
        for (int k = 0; k < 16; ++k) feedbackBlock[k] = cipherData[processed + k];

        processed += 16;
    }

    /*--------------------------------------------------------------
     *  Print results (hexadecimal)
     *--------------------------------------------------------------*/
    std::printf("Key      : ");
    for (int i = 0; i < 16; ++i) std::printf("%02X ", masterKey[i]);
    std::printf("\nIV       : ");
    for (int i = 0; i < 16; ++i) std::printf("%02X ", initVec[i]);
    std::printf("\nPlaintext: ");
    for (int i = 0; i < ptLen; ++i) std::printf("%02X ", plainData[i]);
    std::printf("\nCiphertext: ");
    for (int i = 0; i < ptLen; ++i) std::printf("%02X ", cipherData[i]);
    std::printf("\n");

    return 0;
}
