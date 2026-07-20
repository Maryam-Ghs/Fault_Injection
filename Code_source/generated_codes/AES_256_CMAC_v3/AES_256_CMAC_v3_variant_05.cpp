#include <iostream>

/* LLM input variant 5: duplicate-heavy */

int main()
{
    /*--------------------------------------------------------------
      S‑box and round constants (all int, no const, no unsigned)
    --------------------------------------------------------------*/
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

    int Rcon[15] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36,
                    0x6C,0xD8,0xAB,0x4D,0x9A};

    /*--------------------------------------------------------------
      Allocate key, message and temporary buffers on the heap
    --------------------------------------------------------------*/
    int *key      = new int[32];   // 256‑bit key
    int *msg      = new int[16];   // single 128‑bit block message
    int *zeroBlk  = new int[16];   // all‑zero block for sub‑key generation
    int *K1       = new int[16];   // first CMAC sub‑key
    int *K2       = new int[16];   // second CMAC sub‑key
    int *tmpBlock = new int[16];   // temporary block holder
    int *outBlk   = new int[16];   // AES output block

    /*--------------------------------------------------------------
      Initialise key, message and zero block (duplicate‑heavy arrays)
    --------------------------------------------------------------*/
    for (int i = 0; i < 32; ++i) key[i] = 0x7F;      // all bytes = 127
    for (int i = 0; i < 16; ++i) msg[i] = 0x55;      // all bytes = 85
    for (int i = 0; i < 16; ++i) zeroBlk[i] = 0;

    /*--------------------------------------------------------------
      Helper: multiply by 2 in GF(2^8) (xtime) – re‑ordered arithmetic
    --------------------------------------------------------------*/
    auto xtime = [&](int x) -> int {
        int r = (x << 1) & 0xFF;
        if (x & 0x80) r ^= 0x1B;
        return r;
    };

    /*--------------------------------------------------------------
      AES‑256 key expansion (heap based round‑key storage)
    --------------------------------------------------------------*/
    int *roundKey = new int[240];               // 60 words * 4 bytes
    for (int i = 0; i < 32; ++i) roundKey[i] = key[i];

    for (int i = 8; i < 60; ++i) {
        int t0 = roundKey[(i - 1) * 4 + 0];
        int t1 = roundKey[(i - 1) * 4 + 1];
        int t2 = roundKey[(i - 1) * 4 + 2];
        int t3 = roundKey[(i - 1) * 4 + 3];

        if (i % 8 == 0) {
            // RotWord
            int tmp = t0;
            t0 = t1; t1 = t2; t2 = t3; t3 = tmp;
            // SubWord
            t0 = sbox[t0]; t1 = sbox[t1]; t2 = sbox[t2]; t3 = sbox[t3];
            // Rcon
            t0 ^= Rcon[i / 8 - 1];
        }
        else if (i % 8 == 4) {
            // SubWord only
            t0 = sbox[t0]; t1 = sbox[t1]; t2 = sbox[t2]; t3 = sbox[t3];
        }

        int prev0 = roundKey[(i - 8) * 4 + 0];
        int prev1 = roundKey[(i - 8) * 4 + 1];
        int prev2 = roundKey[(i - 8) * 4 + 2];
        int prev3 = roundKey[(i - 8) * 4 + 3];

        roundKey[i * 4 + 0] = prev0 ^ t0;
        roundKey[i * 4 + 1] = prev1 ^ t1;
        roundKey[i * 4 + 2] = prev2 ^ t2;
        roundKey[i * 4 + 3] = prev3 ^ t3;
    }

    /*--------------------------------------------------------------
      AES‑256 encrypt a single block (in‑place, manual unrolling)
    --------------------------------------------------------------*/
    auto aes256_enc = [&](int *in, int *out) {
        int s[16];
        // copy input to state (unrolled)
        s[0]=in[0];  s[1]=in[1];  s[2]=in[2];  s[3]=in[3];
        s[4]=in[4];  s[5]=in[5];  s[6]=in[6];  s[7]=in[7];
        s[8]=in[8];  s[9]=in[9];  s[10]=in[10];s[11]=in[11];
        s[12]=in[12];s[13]=in[13];s[14]=in[14];s[15]=in[15];

        // Initial AddRoundKey
        for (int i = 0; i < 16; ++i) s[i] ^= roundKey[i];

        // 13 full rounds (Nr‑1 = 13)
        for (int round = 1; round < 14; ++round) {
            // SubBytes
            s[0]=sbox[s[0]]; s[1]=sbox[s[1]]; s[2]=sbox[s[2]]; s[3]=sbox[s[3]];
            s[4]=sbox[s[4]]; s[5]=sbox[s[5]]; s[6]=sbox[s[6]]; s[7]=sbox[s[7]];
            s[8]=sbox[s[8]]; s[9]=sbox[s[9]]; s[10]=sbox[s[10]]; s[11]=sbox[s[11]];
            s[12]=sbox[s[12]]; s[13]=sbox[s[13]]; s[14]=sbox[s[14]]; s[15]=sbox[s[15]];

            // ShiftRows (manual)
            int t1 = s[1];  int t5 = s[5];  int t9 = s[9];  int t13 = s[13];
            s[1] = s[5];    s[5] = s[9];    s[9] = s[13];   s[13] = t1;

            int t2 = s[2];  int t6 = s[6];  int t10 = s[10]; int t14 = s[14];
            s[2] = s[10];   s[6] = s[14];   s[10] = t2;     s[14] = t6;

            int t3 = s[3];  int t7 = s[7];  int t11 = s[11]; int t15 = s[15];
            s[3] = s[15];   s[7] = t3;      s[11] = t7;     s[15] = t11;

            // MixColumns (manual, using xtime)
            for (int c = 0; c < 4; ++c) {
                int i0 = c*4+0, i1 = c*4+1, i2 = c*4+2, i3 = c*4+3;
                int a0 = s[i0], a1 = s[i1], a2 = s[i2], a3 = s[i3];
                int t = a0 ^ a1 ^ a2 ^ a3;
                int u = a0;
                s[i0] ^= t ^ xtime(a0 ^ a1);
                s[i1] ^= t ^ xtime(a1 ^ a2);
                s[i2] ^= t ^ xtime(a2 ^ a3);
                s[i3] ^= t ^ xtime(a3 ^ u);
            }

            // AddRoundKey
            for (int i = 0; i < 16; ++i) s[i] ^= roundKey[round*16 + i];
        }

        // Final round (no MixColumns)
        s[0]=sbox[s[0]]; s[1]=sbox[s[1]]; s[2]=sbox[s[2]]; s[3]=sbox[s[3]];
        s[4]=sbox[s[4]]; s[5]=sbox[s[5]]; s[6]=sbox[s[6]]; s[7]=sbox[s[7]];
        s[8]=sbox[s[8]]; s[9]=sbox[s[9]]; s[10]=sbox[s[10]]; s[11]=sbox[s[11]];
        s[12]=sbox[s[12]]; s[13]=sbox[s[13]]; s[14]=sbox[s[14]]; s[15]=sbox[s[15]];

        // ShiftRows (final)
        int t1 = s[1];  int t5 = s[5];  int t9 = s[9];  int t13 = s[13];
        s[1] = s[5];    s[5] = s[9];    s[9] = s[13];   s[13] = t1;

        int t2 = s[2];  int t6 = s[6];  int t10 = s[10]; int t14 = s[14];
        s[2] = s[10];   s[6] = s[14];   s[10] = t2;     s[14] = t6;

        int t3 = s[3];  int t7 = s[7];  int t11 = s[11]; int t15 = s[15];
        s[3] = s[15];   s[7] = t3;      s[11] = t7;     s[15] = t11;

        // Final AddRoundKey
        for (int i = 0; i < 16; ++i) s[i] ^= roundKey[14*16 + i];

        // copy to output (unrolled)
        out[0]=s[0]; out[1]=s[1]; out[2]=s[2]; out[3]=s[3];
        out[4]=s[4]; out[5]=s[5]; out[6]=s[6]; out[7]=s[7];
        out[8]=s[8]; out[9]=s[9]; out[10]=s[10]; out[11]=s[11];
        out[12]=s[12]; out[13]=s[13]; out[14]=s[14]; out[15]=s[15];
    };

    /*--------------------------------------------------------------
      Generate sub‑keys K1 and K2
    --------------------------------------------------------------*/
    aes256_enc(zeroBlk, tmpBlock);            // L = AES_K(0^128)

    // Left shift L by 1 bit (manual, unrolled)
    int carry = 0;
    for (int i = 15; i >= 0; --i) {
        int newCarry = (tmpBlock[i] & 0x80) ? 1 : 0;
        K1[i] = ((tmpBlock[i] << 1) & 0xFF) | carry;
        carry = newCarry;
    }
    if (carry) K1[0] ^= 0x87;                // Rb

    // K2 = leftshift(K1) (unrolled)
    carry = 0;
    for (int i = 15; i >= 0; --i) {
        int newCarry = (K1[i] & 0x80) ? 1 : 0;
        K2[i] = ((K1[i] << 1) & 0xFF) | carry;
        carry = newCarry;
    }
    if (carry) K2[0] ^= 0x87;

    /*--------------------------------------------------------------
      CMAC calculation (single‑block message => use K1)
    --------------------------------------------------------------*/
    // M_last = M XOR K1
    for (int i = 0; i < 16; ++i) tmpBlock[i] = msg[i] ^ K1[i];

    // CMAC = AES_K(M_last)
    aes256_enc(tmpBlock, outBlk);

    /*--------------------------------------------------------------
      Print CMAC as hex string
    --------------------------------------------------------------*/
    std::cout << "CMAC = ";
    for (int i = 0; i < 16; ++i) {
        int hi = (outBlk[i] >> 4) & 0xF;
        int lo = outBlk[i] & 0xF;
        char hch = (hi < 10) ? ('0' + hi) : ('A' + hi - 10);
        char lch = (lo < 10) ? ('0' + lo) : ('A' + lo - 10);
        std::cout << hch << lch;
    }
    std::cout << std::endl;

    /*--------------------------------------------------------------
      Clean up heap memory
    --------------------------------------------------------------*/
    delete[] key;
    delete[] msg;
    delete[] zeroBlk;
    delete[] K1;
    delete[] K2;
    delete[] tmpBlock;
    delete[] outBlk;
    delete[] roundKey;

    return 0;
}
