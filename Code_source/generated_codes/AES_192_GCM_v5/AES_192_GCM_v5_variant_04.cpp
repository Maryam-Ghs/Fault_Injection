#include <iostream>

/* LLM input variant 4: signed-extremes */

int main()
{
    /*--------------------------------------------------------------
     *  Version #5 – AES‑192‑GCM (single block, deterministic test)
     *  All code lives inside main(), uses only int and float types.
     *  No const, no unsigned, no long, no double, no cin.
     *--------------------------------------------------------------*/

    /*------------------------ Test vectors -----------------------*/
    int keyBytes[24] = {
        -0x10, 0x01, -0x02, 0x03,
        0x04, -0x05, 0x06, -0x07,
        0x08, 0x00, -0x0A, 0x0B,
        -0x0C, 0x0D, 0x0E, -0x0F,
        0x10, -0x11, 0x12, 0x00,
        -0x14, 0x15, 0x00, -0x17
    };                                          // 192‑bit key

    int ivBytes[12] = {                         // 96‑bit IV
        0xa0, 0x00, 0xa2, 0xa3,
        0xa4, 0x00, 0xa6, 0xa7,
        0x00, 0xa9, 0xaa, 0x00
    };

    int plain[16] = {                           // one block plaintext
        -0x20, 0x21, 0x00, -0x23,
        0x24, -0x25, 0x26, 0x00,
        -0x28, 0x00, 0x2A, -0x2B,
        0x00, -0x2D, 0x2E, 0x2F
    };

    /*--------------------------- S‑Box ---------------------------*/
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

    /*-------------------------- Rcon ---------------------------*/
    int rcon[10] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36};

    /*---------------------- Round keys ---------------------------*/
    int roundKey[13][16];                     // 13 round keys, each 16 bytes

    /*--------------------- Key expansion ------------------------*/
    // copy original key into first 24 bytes of roundKey[0]
    for (int i = 0; i < 24; ++i)
        roundKey[0][i] = keyBytes[i];
    // remaining 8 bytes of first round key are derived
    for (int i = 24; i < 32; ++i)
        roundKey[0][i] = 0;

    // AES‑192 needs 12 rounds → 13 round keys
    int temp[4];
    int generated = 6;                       // words generated (6 words = 24 bytes)
    int rconIdx = 0;

    while (generated < 52)                    // 52 words total
    {
        // take previous word (4 bytes)
        for (int b = 0; b < 4; ++b)
            temp[b] = roundKey[(generated-1)/4][((generated-1)%4)*4 + b];

        // every 6th word, apply core schedule
        if (generated % 6 == 0)
        {
            // RotWord
            int rot = temp[0];
            temp[0] = temp[1];
            temp[1] = temp[2];
            temp[2] = temp[3];
            temp[3] = rot;

            // SubWord
            for (int b = 0; b < 4; ++b)
                temp[b] = sbox[temp[b] & 0xFF];

            // Rcon
            temp[0] ^= rcon[rconIdx];
            ++rconIdx;
        }
        // every 6th word + 4, also SubWord (but not RotWord)
        else if (generated % 6 == 4)
        {
            for (int b = 0; b < 4; ++b)
                temp[b] = sbox[temp[b] & 0xFF];
        }

        // XOR with word Nk positions back (6 words back)
        int srcIdx = generated - 6;
        for (int b = 0; b < 4; ++b)
        {
            int srcByte = roundKey[srcIdx/4][(srcIdx%4)*4 + b];
            temp[b] ^= srcByte;
        }

        // store new word
        int dstWord = generated;
        for (int b = 0; b < 4; ++b)
            roundKey[dstWord/4][(dstWord%4)*4 + b] = temp[b];

        ++generated;
    }

    /*--------------------- Helper: AddRoundKey ------------------*/
    auto addRoundKey = [&](int state[4][4], int rkIdx)
    {
        for (int c = 0; c < 4; ++c)
            for (int r = 0; r < 4; ++r)
            {
                int byte = roundKey[rkIdx][c*4 + r];
                state[r][c] ^= byte;
            }
    };

    /*--------------------- Helper: SubBytes --------------------*/
    auto subBytes = [&](int state[4][4])
    {
        for (int c = 0; c < 4; ++c)
            for (int r = 0; r < 4; ++r)
                state[r][c] = sbox[state[r][c] & 0xFF];
    };

    /*--------------------- Helper: ShiftRows -------------------*/
    auto shiftRows = [&](int state[4][4])
    {
        int tmp;

        // row 1 left 1
        tmp = state[1][0];
        for (int c = 0; c < 3; ++c) state[1][c] = state[1][c+1];
        state[1][3] = tmp;

        // row 2 left 2
        tmp = state[2][0];
        state[2][0] = state[2][2];
        state[2][2] = tmp;
        tmp = state[2][1];
        state[2][1] = state[2][3];
        state[2][3] = tmp;

        // row 3 left 3 (right 1)
        tmp = state[3][3];
        for (int c = 3; c > 0; --c) state[3][c] = state[3][c-1];
        state[3][0] = tmp;
    };

    /*--------------------- Helper: MixColumns -------------------*/
    auto xtime = [&](int x)
    {
        int hi = (x & 0x80) ? 0x1B : 0;
        return ((x << 1) & 0xFF) ^ hi;
    };

    auto mixColumns = [&](int state[4][4])
    {
        for (int c = 0; c < 4; ++c)
        {
            int a0 = state[0][c];
            int a1 = state[1][c];
            int a2 = state[2][c];
            int a3 = state[3][c];

            int r0 = xtime(a0) ^ (xtime(a1) ^ a1) ^ a2 ^ a3;
            int r1 = a0 ^ xtime(a1) ^ (xtime(a2) ^ a2) ^ a3;
            int r2 = a0 ^ a1 ^ xtime(a2) ^ (xtime(a3) ^ a3);
            int r3 = (xtime(a0) ^ a0) ^ a1 ^ a2 ^ xtime(a3);

            state[0][c] = r0 & 0xFF;
            state[1][c] = r1 & 0xFF;
            state[2][c] = r2 & 0xFF;
            state[3][c] = r3 & 0xFF;
        }
    };

    /*---------------------- AES‑192 encrypt ---------------------*/
    auto aesEncryptBlock = [&](int in[16], int out[16])
    {
        int state[4][4];
        // map input bytes into state (column‑major)
        for (int i = 0; i < 16; ++i)
            state[i%4][i/4] = in[i] & 0xFF;

        // initial round key
        addRoundKey(state, 0);

        // 11 full rounds
        for (int round = 1; round < 12; ++round)
        {
            subBytes(state);
            shiftRows(state);
            mixColumns(state);
            addRoundKey(state, round);
        }

        // final round (no MixColumns)
        subBytes(state);
        shiftRows(state);
        addRoundKey(state, 12);

        // write back to out
        for (int i = 0; i < 16; ++i)
            out[i] = state[i%4][i/4] & 0xFF;
    };

    /*------------------- GCM helper: GHASH ---------------------*/
    // multiplication in GF(2^128) with the polynomial x^128 + x^7 + x^2 + x + 1
    auto ghashMul = [&](int X[4], int Y[4], int Z[4])
    {
        int V[4];
        for (int i = 0; i < 4; ++i) V[i] = Y[i];
        for (int i = 0; i < 4; ++i) Z[i] = 0;

        for (int i = 0; i < 128; ++i)
        {
            int bit = (X[i/32] >> (31 - (i%32))) & 1;
            if (bit)
                for (int w = 0; w < 4; ++w) Z[w] ^= V[w];

            // shift V right by 1 (as a 128‑bit number)
            int lsb = V[3] & 1;
            for (int w = 3; w > 0; --w) V[w] = (V[w] >> 1) | ((V[w-1] & 1) << 31);
            V[0] >>= 1;

            // if lsb was 1, xor reduction polynomial
            if (lsb)
            {
                V[0] ^= 0xE1000000;   // 0b1110 0001 followed by 24 zeros
            }
        }
    };

    /*------------------- GCM processing ------------------------*/
    // 1) Compute hash subkey H = AES_K(0^128)
    int zeroBlock[16] = {0};
    int Hbytes[16];
    aesEncryptBlock(zeroBlock, Hbytes);
    int H[4];
    for (int i = 0; i < 4; ++i)
        H[i] = (Hbytes[i*4] << 24) | (Hbytes[i*4+1] << 16) |
               (Hbytes[i*4+2] << 8) | Hbytes[i*4+3];

    // 2) Encrypt plaintext (counter mode)
    // Build J0 = IV || 0x00000001 (since IV is 96‑bit)
    int J0[4] = {0};
    J0[0] = (ivBytes[0] << 24) | (ivBytes[1] << 16) | (ivBytes[2] << 8) | ivBytes[3];
    J0[1] = (ivBytes[4] << 24) | (ivBytes[5] << 16) | (ivBytes[6] << 8) | ivBytes[7];
    J0[2] = (ivBytes[8] << 24) | (ivBytes[9] << 16) | (ivBytes[10] << 8) | ivBytes[11];
    J0[3] = 1;                                   // low 32 bits = 1

    // increment counter for the block
    int ctr[4];
    for (int i = 0; i < 4; ++i) ctr[i] = J0[i];
    // increment low word
    ctr[3] = (ctr[3] + 1) & 0xFFFFFFFF;

    // encrypt counter
    int ctrBytes[16];
    for (int i = 0; i < 4; ++i)
    {
        ctrBytes[i*4]   = (ctr[i] >> 24) & 0xFF;
        ctrBytes[i*4+1] = (ctr[i] >> 16) & 0xFF;
        ctrBytes[i*4+2] = (ctr[i] >> 8) & 0xFF;
        ctrBytes[i*4+3] = ctr[i] & 0xFF;
    }
    int keystream[16];
    aesEncryptBlock(ctrBytes, keystream);

    int cipher[16];
    for (int i = 0; i < 16; ++i)
        cipher[i] = plain[i] ^ keystream[i];

    // 3) GHASH over ciphertext (no AAD, single block)
    int Y[4] = {0,0,0,0};
    int Cblock[4];
    for (int i = 0; i < 4; ++i)
        Cblock[i] = (cipher[i*4] << 24) | (cipher[i*4+1] << 16) |
                    (cipher[i*4+2] << 8) | cipher[i*4+3];

    // Y = (Y xor C) * H
    for (int i = 0; i < 4; ++i) Y[i] ^= Cblock[i];
    int tmpZ[4];
    ghashMul(Y, H, tmpZ);
    for (int i = 0; i < 4; ++i) Y[i] = tmpZ[i];

    // 4) Compute authentication tag: T = AES_K(J0) xor Y
    int J0bytes[16];
    for (int i = 0; i < 4; ++i)
    {
        J0bytes[i*4]   = (J0[i] >> 24) & 0xFF;
        J0bytes[i*4+1] = (J0[i] >> 16) & 0xFF;
        J0bytes[i*4+2] = (J0[i] >> 8) & 0xFF;
        J0bytes[i*4+3] = J0[i] & 0xFF;
    }
    int S[16];
    aesEncryptBlock(J0bytes, S);
    int tag[16];
    for (int i = 0; i < 16; ++i) tag[i] = S[i] ^ ((Y[i/4] >> ((3-(i%4))*8)) & 0xFF);

    /*--------------------------- Output --------------------------*/
    std::cout << "Ciphertext: ";
    for (int i = 0; i < 16; ++i)
        std::cout << std::hex << (cipher[i] & 0xFF);
    std::cout << std::dec << "\n";

    std::cout << "Tag: ";
    for (int i = 0; i < 16; ++i)
        std::cout << std::hex << (tag[i] & 0xFF);
    std::cout << std::dec << "\n";

    return 0;
}
