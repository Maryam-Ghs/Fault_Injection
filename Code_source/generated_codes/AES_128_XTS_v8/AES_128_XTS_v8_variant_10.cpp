#include <iostream>
#include <iomanip>

int main()
{
    /* LLM input variant 10: large-safe-stress */

    /* ------------------------------------------------------------------ */
    /* 1.  Fixed data (small predefined arrays)                            */
    /* ------------------------------------------------------------------ */
    const int NUM_BLOCKS = 1024;                // stress test with 1024 blocks
    int ptBlk[16 * NUM_BLOCKS];
    for (int i = 0; i < 16 * NUM_BLOCKS; ++i) ptBlk[i] = i & 0xff;   // deterministic pattern

    int encKey[16] = {                                      // AES‑128 key K1
        0x2b,0x7e,0x15,0x16,0x28,0xae,0xd2,0xa6,
        0xab,0xf7,0x15,0x88,0x09,0xcf,0x4f,0x3c
    };
    int twkKey[16] = {                                      // AES‑128 key K2 (tweak)
        0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
        0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f
    };

    /* ------------------------------------------------------------------ */
    /* 2.  Tables (int only, no const)                                    */
    /* ------------------------------------------------------------------ */
    int sboxTbl[256] = {
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
    int rconTbl[11] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36,0x6C};

    /* ------------------------------------------------------------------ */
    /* 3.  Helper lambdas (still inside main)                              */
    /* ------------------------------------------------------------------ */
    auto xtime = [](int v)->int{
        int r = v << 1;
        if (v & 0x80) r ^= 0x1b;
        return r & 0xff;
    };
    auto mul2 = [&](int v){ return xtime(v); };
    auto mul3 = [&](int v){ return xtime(v) ^ v; };

    /* ------------------------------------------------------------------ */
    /* 4.  Key schedule for a 128‑bit key (produces 11 round keys)         */
    /* ------------------------------------------------------------------ */
    int roundKey[11][16];
    for (int i = 0; i < 16; ++i) roundKey[0][i] = encKey[i];

    for (int r = 1; r <= 10; ++r)
    {
        int temp0 = roundKey[r-1][13];
        int temp1 = roundKey[r-1][14];
        int temp2 = roundKey[r-1][15];
        int temp3 = roundKey[r-1][12];

        /* SubWord + RotWord */
        temp0 = sboxTbl[temp0];
        temp1 = sboxTbl[temp1];
        temp2 = sboxTbl[temp2];
        temp3 = sboxTbl[temp3];

        temp0 ^= rconTbl[r-1];

        for (int i = 0; i < 4; ++i)
        {
            roundKey[r][i] = roundKey[r-1][i] ^ temp0;
            roundKey[r][i+4] = roundKey[r-1][i+4] ^ roundKey[r][i];
            roundKey[r][i+8] = roundKey[r-1][i+8] ^ roundKey[r][i+4];
            roundKey[r][i+12] = roundKey[r-1][i+12] ^ roundKey[r][i+8];
            temp0 = roundKey[r-1][i+4];
            temp1 = roundKey[r-1][i+8];
            temp2 = roundKey[r-1][i+12];
            temp3 = roundKey[r-1][i];
        }
    }

    /* ------------------------------------------------------------------ */
    /* 5.  Compute tweak: encrypt a zero block with the tweak key          */
    /* ------------------------------------------------------------------ */
    int zeroBlk[16] = {0};
    int wkRound[11][16];
    for (int i = 0; i < 16; ++i) wkRound[0][i] = twkKey[i];

    for (int r = 1; r <= 10; ++r)
    {
        int t0 = wkRound[r-1][13];
        int t1 = wkRound[r-1][14];
        int t2 = wkRound[r-1][15];
        int t3 = wkRound[r-1][12];

        t0 = sboxTbl[t0];
        t1 = sboxTbl[t1];
        t2 = sboxTbl[t2];
        t3 = sboxTbl[t3];

        t0 ^= rconTbl[r-1];

        for (int i = 0; i < 4; ++i)
        {
            wkRound[r][i] = wkRound[r-1][i] ^ t0;
            wkRound[r][i+4] = wkRound[r-1][i+4] ^ wkRound[r][i];
            wkRound[r][i+8] = wkRound[r-1][i+8] ^ wkRound[r][i+4];
            wkRound[r][i+12] = wkRound[r-1][i+12] ^ wkRound[r][i+8];
            t0 = wkRound[r-1][i+4];
            t1 = wkRound[r-1][i+8];
            t2 = wkRound[r-1][i+12];
            t3 = wkRound[r-1][i];
        }
    }

    /* encrypt zero block with wkRound keys (manual round unrolling) */
    int tweakBlk[16];
    for (int i = 0; i < 16; ++i) tweakBlk[i] = zeroBlk[i] ^ wkRound[0][i];

    /* ---- Round 1 ----------------------------------------------------- */
    for (int i = 0; i < 16; ++i) tweakBlk[i] = sboxTbl[tweakBlk[i]];
    /* ShiftRows */
    int tmp0 = tweakBlk[1]; tweakBlk[1] = tweakBlk[5]; tweakBlk[5] = tweakBlk[9];
    tweakBlk[9] = tweakBlk[13]; tweakBlk[13] = tmp0;
    int tmp1 = tweakBlk[2]; int tmp2 = tweakBlk[6];
    tweakBlk[2] = tweakBlk[10]; tweakBlk[6] = tweakBlk[14];
    tweakBlk[10] = tmp1; tweakBlk[14] = tmp2;
    tmp0 = tweakBlk[3]; tmp1 = tweakBlk[7]; tmp2 = tweakBlk[11];
    tweakBlk[3] = tweakBlk[15]; tweakBlk[7] = tmp0;
    tweakBlk[11] = tmp1; tweakBlk[15] = tmp2;
    /* MixColumns */
    for (int c = 0; c < 4; ++c)
    {
        int i0 = c*4, i1 = i0+1, i2 = i0+2, i3 = i0+3;
        int a0 = tweakBlk[i0], a1 = tweakBlk[i1], a2 = tweakBlk[i2], a3 = tweakBlk[i3];
        int m0 = mul2(a0) ^ mul3(a1) ^ a2 ^ a3;
        int m1 = a0 ^ mul2(a1) ^ mul3(a2) ^ a3;
        int m2 = a0 ^ a1 ^ mul2(a2) ^ mul3(a3);
        int m3 = mul3(a0) ^ a1 ^ a2 ^ mul2(a3);
        tweakBlk[i0]=m0; tweakBlk[i1]=m1; tweakBlk[i2]=m2; tweakBlk[i3]=m3;
    }
    for (int i = 0; i < 16; ++i) tweakBlk[i] ^= wkRound[1][i];

    /* ---- Rounds 2 … 9 ------------------------------------------------- */
    for (int r = 2; r <= 9; ++r)
    {
        for (int i = 0; i < 16; ++i) tweakBlk[i] = sboxTbl[tweakBlk[i]];
        /* ShiftRows */
        tmp0 = tweakBlk[1]; tweakBlk[1] = tweakBlk[5]; tweakBlk[5] = tweakBlk[9];
        tweakBlk[9] = tweakBlk[13]; tweakBlk[13] = tmp0;
        tmp0 = tweakBlk[2]; tmp1 = tweakBlk[6];
        tweakBlk[2] = tweakBlk[10]; tweakBlk[6] = tweakBlk[14];
        tweakBlk[10] = tmp0; tweakBlk[14] = tmp1;
        tmp0 = tweakBlk[3]; tmp1 = tweakBlk[7]; tmp2 = tweakBlk[11];
        tweakBlk[3] = tweakBlk[15]; tweakBlk[7] = tmp0;
        tweakBlk[11] = tmp1; tweakBlk[15] = tmp2;
        /* MixColumns */
        for (int c = 0; c < 4; ++c)
        {
            int i0 = c*4, i1 = i0+1, i2 = i0+2, i3 = i0+3;
            int a0 = tweakBlk[i0], a1 = tweakBlk[i1], a2 = tweakBlk[i2], a3 = tweakBlk[i3];
            int m0 = mul2(a0) ^ mul3(a1) ^ a2 ^ a3;
            int m1 = a0 ^ mul2(a1) ^ mul3(a2) ^ a3;
            int m2 = a0 ^ a1 ^ mul2(a2) ^ mul3(a3);
            int m3 = mul3(a0) ^ a1 ^ a2 ^ mul2(a3);
            tweakBlk[i0]=m0; tweakBlk[i1]=m1; tweakBlk[i2]=m2; tweakBlk[i3]=m3;
        }
        for (int i = 0; i < 16; ++i) tweakBlk[i] ^= wkRound[r][i];
    }

    /* ---- Final round (no MixColumns) ----------------------------------- */
    for (int i = 0; i < 16; ++i) tweakBlk[i] = sboxTbl[tweakBlk[i]];
    /* ShiftRows */
    tmp0 = tweakBlk[1]; tweakBlk[1] = tweakBlk[5]; tweakBlk[5] = tweakBlk[9];
    tweakBlk[9] = tweakBlk[13]; tweakBlk[13] = tmp0;
    tmp0 = tweakBlk[2]; tmp1 = tweakBlk[6];
    tweakBlk[2] = tweakBlk[10]; tweakBlk[6] = tweakBlk[14];
    tweakBlk[10] = tmp0; tweakBlk[14] = tmp1;
    tmp0 = tweakBlk[3]; tmp1 = tweakBlk[7]; tmp2 = tweakBlk[11];
    tweakBlk[3] = tweakBlk[15]; tweakBlk[7] = tmp0;
    tweakBlk[11] = tmp1; tweakBlk[15] = tmp2;
    for (int i = 0; i < 16; ++i) tweakBlk[i] ^= wkRound[10][i];

    /* ------------------------------------------------------------------ */
    /* 6.  Encrypt all blocks (XTS mode, re‑using same tweak for stress)   */
    /* ------------------------------------------------------------------ */
    int ctBlk[16 * NUM_BLOCKS];

    // lambda that encrypts a single 16‑byte block using pre‑computed roundKey and tweak
    auto encrypt_one = [&](int in[16], int out[16]){
        int work[16];
        for (int i = 0; i < 16; ++i) work[i] = in[i] ^ tweakBlk[i];   // pre‑whiten

        // Round 1
        for (int i = 0; i < 16; ++i) work[i] = sboxTbl[work[i]];
        // ShiftRows
        tmp0 = work[1]; work[1] = work[5]; work[5] = work[9];
        work[9] = work[13]; work[13] = tmp0;
        tmp0 = work[2]; tmp1 = work[6];
        work[2] = work[10]; work[6] = work[14];
        work[10] = tmp0; work[14] = tmp1;
        tmp0 = work[3]; tmp1 = work[7]; tmp2 = work[11];
        work[3] = work[15]; work[7] = tmp0;
        work[11] = tmp1; work[15] = tmp2;
        // MixColumns
        for (int c = 0; c < 4; ++c)
        {
            int i0 = c*4, i1 = i0+1, i2 = i0+2, i3 = i0+3;
            int a0 = work[i0], a1 = work[i1], a2 = work[i2], a3 = work[i3];
            int m0 = mul2(a0) ^ mul3(a1) ^ a2 ^ a3;
            int m1 = a0 ^ mul2(a1) ^ mul3(a2) ^ a3;
            int m2 = a0 ^ a1 ^ mul2(a2) ^ mul3(a3);
            int m3 = mul3(a0) ^ a1 ^ a2 ^ mul2(a3);
            work[i0]=m0; work[i1]=m1; work[i2]=m2; work[i3]=m3;
        }
        for (int i = 0; i < 16; ++i) work[i] ^= roundKey[1][i];

        // Rounds 2 … 9
        for (int r = 2; r <= 9; ++r)
        {
            for (int i = 0; i < 16; ++i) work[i] = sboxTbl[work[i]];
            // ShiftRows
            tmp0 = work[1]; work[1] = work[5]; work[5] = work[9];
            work[9] = work[13]; work[13] = tmp0;
            tmp0 = work[2]; tmp1 = work[6];
            work[2] = work[10]; work[6] = work[14];
            work[10] = tmp0; work[14] = tmp1;
            tmp0 = work[3]; tmp1 = work[7]; tmp2 = work[11];
            work[3] = work[15]; work[7] = tmp0;
            work[11] = tmp1; work[15] = tmp2;
            // MixColumns
            for (int c = 0; c < 4; ++c)
            {
                int i0 = c*4, i1 = i0+1, i2 = i0+2, i3 = i0+3;
                int a0 = work[i0], a1 = work[i1], a2 = work[i2], a3 = work[i3];
                int m0 = mul2(a0) ^ mul3(a1) ^ a2 ^ a3;
                int m1 = a0 ^ mul2(a1) ^ mul3(a2) ^ a3;
                int m2 = a0 ^ a1 ^ mul2(a2) ^ mul3(a3);
                int m3 = mul3(a0) ^ a1 ^ a2 ^ mul2(a3);
                work[i0]=m0; work[i1]=m1; work[i2]=m2; work[i3]=m3;
            }
            for (int i = 0; i < 16; ++i) work[i] ^= roundKey[r][i];
        }

        // Final round (no MixColumns)
        for (int i = 0; i < 16; ++i) work[i] = sboxTbl[work[i]];
        // ShiftRows
        tmp0 = work[1]; work[1] = work[5]; work[5] = work[9];
        work[9] = work[13]; work[13] = tmp0;
        tmp0 = work[2]; tmp1 = work[6];
        work[2] = work[10]; work[6] = work[14];
        work[10] = tmp0; work[14] = tmp1;
        tmp0 = work[3]; tmp1 = work[7]; tmp2 = work[11];
        work[3] = work[15]; work[7] = tmp0;
        work[11] = tmp1; work[15] = tmp2;
        for (int i = 0; i < 16; ++i) work[i] ^= roundKey[10][i];

        // post‑whiten
        for (int i = 0; i < 16; ++i) out[i] = work[i] ^ tweakBlk[i];
    };

    for (int b = 0; b < NUM_BLOCKS; ++b)
    {
        encrypt_one(&ptBlk[16*b], &ctBlk[16*b]);
    }

    /* ------------------------------------------------------------------ */
    /* 7.  Output (hex) – show first block and total block count           */
    /* ------------------------------------------------------------------ */
    std::cout << "Processed " << NUM_BLOCKS << " blocks.\n";
    std::cout << "First plaintext block : ";
    for (int i = 0; i < 16; ++i)
        std::cout << std::hex << std::setw(2) << std::setfill('0') << ptBlk[i] << ' ';
    std::cout << "\nFirst ciphertext block: ";
    for (int i = 0; i < 16; ++i)
        std::cout << std::hex << std::setw(2) << std::setfill('0') << ctBlk[i] << ' ';
    std::cout << std::dec << std::endl;

    return 0;
}
