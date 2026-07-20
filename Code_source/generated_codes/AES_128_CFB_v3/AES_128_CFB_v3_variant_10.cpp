#include <cstdio>
#include <cstdlib>

/* LLM input variant 10: large-safe-stress */

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
     *  Random key, IV and plaintext (larger arrays for stress)
     *--------------------------------------------------------------*/
    int masterKey[16];
    int idx = 0;
    while (idx < 16) { masterKey[idx] = std::rand() & 0xFF; ++idx; }

    int initVec[16];
    idx = 0;
    while (idx < 16) { initVec[idx] = std::rand() & 0xFF; ++idx; }

    const int ptLen = 1024;                 // 64 AES blocks
    int plainData[ptLen];
    idx = 0;
    while (idx < ptLen) { plainData[idx] = std::rand() & 0xFF; ++idx; }

    /*--------------------------------------------------------------
     *  Key expansion (AES‑128 => 11 round keys, 176 bytes)
     *--------------------------------------------------------------*/
    int roundKeys[11][16];
    idx = 0;
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
        for (int i = 0; i < 16; ++i) state[i] ^= roundKeys[0][i];

        /* ---------- Rounds 1 … 9 ---------- */
        int round = 1;
        while (round <= 9)
        {
            /* SubBytes */
            for (int i = 0; i < 16; ++i) state[i] = sboxArr[state[i]];

            /* ShiftRows (manual mapping) */
            int tmp1 = state[1];  int tmp5 = state[5];  int tmp9 = state[9];  int tmp13 = state[13];
            state[1]  = tmp5;   state[5]  = tmp9;
            state[9]  = tmp13;  state[13] = tmp1;

            int tmp2 = state[2];   int tmp6 = state[6];
            int tmp10 = state[10]; int tmp14 = state[14];
            state[2]  = tmp10;  state[6]  = tmp14;
            state[10] = tmp2;   state[14] = tmp6;

            int tmp3 = state[3];   int tmp7 = state[7];
            int tmp11 = state[11]; int tmp15 = state[15];
            state[3]  = tmp15;  state[7]  = tmp3;
            state[11] = tmp7;   state[15] = tmp11;

            /* MixColumns (expanded multi‑step) */
            for (int col = 0; col < 4; ++col)
            {
                int base = col * 4;
                int c0 = state[base];
                int c1 = state[base+1];
                int c2 = state[base+2];
                int c3 = state[base+3];
                int m0 = mul2(c0) ^ mul3(c1) ^ c2 ^ c3;
                int m1 = c0 ^ mul2(c1) ^ mul3(c2) ^ c3;
                int m2 = c0 ^ c1 ^ mul2(c2) ^ mul3(c3);
                int m3 = mul3(c0) ^ c1 ^ c2 ^ mul2(c3);
                state[base]   = m0;
                state[base+1] = m1;
                state[base+2] = m2;
                state[base+3] = m3;
            }

            /* AddRoundKey */
            for (int i = 0; i < 16; ++i) state[i] ^= roundKeys[round][i];

            ++round;
        }

        /* ---------- Final round (no MixColumns) ---------- */
        /* SubBytes */
        for (int i = 0; i < 16; ++i) state[i] = sboxArr[state[i]];

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
        for (int i = 0; i < 16; ++i) state[i] ^= roundKeys[10][i];

        /* copy to output */
        for (int i = 0; i < 16; ++i) out[i] = state[i];
    };

    /*--------------------------------------------------------------
     *  CFB‑128 encryption (feedback = ciphertext)
     *--------------------------------------------------------------*/
    int cipherData[ptLen];
    int feedbackBlock[16];
    for (int i = 0; i < 16; ++i) feedbackBlock[i] = initVec[i];

    int processed = 0;
    while (processed < ptLen)
    {
        int keystream[16];
        encryptBlock(feedbackBlock, keystream);

        /* XOR with current plaintext block */
        for (int j = 0; j < 16; ++j)
            cipherData[processed + j] = (keystream[j] ^ plainData[processed + j]) & 0xFF;

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
