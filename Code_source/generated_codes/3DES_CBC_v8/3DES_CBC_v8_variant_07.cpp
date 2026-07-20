/*********************************************************************
 * 3DES-CBC implementation (version #8)
 *  - Only int and float are used (no double, long, unsigned, const)
 *  - All data are stored in stack arrays
 *  - Edge‑case heavy test vectors are generated inside main()
 *  - Functions are used, but the whole program flow stays inside main()
 *********************************************************************/

#include <iostream>
#include <iomanip>

/* LLM input variant 7: reverse-adversarial */

/* ----------------------------------------------------------------- */
/*  Permutation tables (non‑const, int only)                         */
/* ----------------------------------------------------------------- */
int IP[64] = {
    58,50,42,34,26,18,10,2,
    60,52,44,36,28,20,12,4,
    62,54,46,38,30,22,14,6,
    64,56,48,40,32,24,16,8,
    57,49,41,33,25,17,9,1,
    59,51,43,35,27,19,11,3,
    61,53,45,37,29,21,13,5,
    63,55,47,39,31,23,15,7
};

int FP[64] = {
    40,8,48,16,56,24,64,32,
    39,7,47,15,55,23,63,31,
    38,6,46,14,54,22,62,30,
    37,5,45,13,53,21,61,29,
    36,4,44,12,52,20,60,28,
    35,3,43,11,51,19,59,27,
    34,2,42,10,50,18,58,26,
    33,1,41,9,49,17,57,25
};

int E[48] = {
    32,1,2,3,4,5,
    4,5,6,7,8,9,
    8,9,10,11,12,13,
    12,13,14,15,16,17,
    16,17,18,19,20,21,
    20,21,22,23,24,25,
    24,25,26,27,28,29,
    28,29,30,31,32,1
};

int P[32] = {
    16,7,20,21,
    29,12,28,17,
    1,15,23,26,
    5,18,31,10,
    2,8,24,14,
    32,27,3,9,
    19,13,30,6,
    22,11,4,25
};

int PC1[56] = {
    57,49,41,33,25,17,9,
    1,58,50,42,34,26,18,
    10,2,59,51,43,35,27,
    19,11,3,60,52,44,36,
    63,55,47,39,31,23,15,
    7,62,54,46,38,30,22,
    14,6,61,53,45,37,29,
    21,13,5,28,20,12,4
};

int PC2[48] = {
    14,17,11,24,1,5,
    3,28,15,6,21,10,
    23,19,12,4,26,8,
    16,7,27,20,13,2,
    41,52,31,37,47,55,
    30,40,51,45,33,48,
    44,49,39,56,34,53,
    46,42,50,36,29,32
};

int SHIFTS[16] = {
    1,1,2,2,2,2,2,2,
    1,2,2,2,2,2,2,1
};

/* S‑boxes (8 × 4 × 16) */
int SBOX[8][4][16] = {
    { {14,4,13,1,2,15,11,8,3,10,6,12,5,9,0,7},
      {0,15,7,4,14,2,13,1,10,6,12,11,9,5,3,8},
      {4,1,14,8,13,6,2,11,15,12,9,7,3,10,5,0},
      {15,12,8,2,4,9,1,7,5,11,3,14,10,0,6,13} },

    { {15,1,8,14,6,11,3,4,9,7,2,13,12,0,5,10},
      {3,13,4,7,15,2,8,14,12,0,1,10,6,9,11,5},
      {0,14,7,11,10,4,13,1,5,8,12,6,9,3,2,15},
      {13,8,10,1,3,15,4,2,11,6,7,12,0,5,14,9} },

    { {10,0,9,14,6,3,15,5,1,13,12,7,11,4,2,8},
      {13,7,0,9,3,4,6,10,2,8,5,14,12,11,15,1},
      {13,6,4,9,8,15,3,0,11,1,2,12,5,10,14,7},
      {1,10,13,0,6,9,8,7,4,15,14,3,11,5,2,12} },

    { {7,13,14,3,0,6,9,10,1,2,8,5,11,12,4,15},
      {13,8,11,5,6,15,0,3,4,7,2,12,1,10,14,9},
      {10,6,9,0,12,11,7,13,15,1,3,14,5,2,8,4},
      {3,15,0,6,10,1,13,8,9,4,5,11,12,7,2,14} },

    { {2,12,4,1,7,10,11,6,8,5,3,15,13,0,14,9},
      {14,11,2,12,4,7,13,1,5,0,15,10,3,9,8,6},
      {4,2,1,11,10,13,7,8,15,9,12,5,6,3,0,14},
      {11,8,12,7,1,14,2,13,6,15,0,9,10,4,5,3} },

    { {12,1,10,15,9,2,6,8,0,13,3,4,14,7,5,11},
      {10,15,4,2,7,12,9,5,6,1,13,14,0,11,3,8},
      {9,14,15,5,2,8,12,3,7,0,4,10,1,13,11,6},
      {4,3,2,12,9,5,15,10,11,14,1,7,6,0,8,13} },

    { {4,11,2,14,15,0,8,13,3,12,9,7,5,10,6,1},
      {13,0,11,7,4,9,1,10,14,3,5,12,2,15,8,6},
      {1,4,11,13,12,3,7,14,10,15,6,8,0,5,9,2},
      {6,11,13,8,1,4,10,7,9,5,0,15,14,2,3,12} }
};

/* ----------------------------------------------------------------- */
/*  Helper functions                                                  */
/* ----------------------------------------------------------------- */

/* copy n bits from src to dst (both bit arrays) */
void copyBits(const int *src, int *dst, int n)
{
    for (int i = 0; i < n; ++i)
        dst[i] = src[i];
}

/* Convert a byte array (0‑255) to a bit array (MSB first) */
void bytesToBits(const int *bytes, int byteCount, int *bits)
{
    for (int i = 0; i < byteCount; ++i)
    {
        int val = bytes[i];
        for (int b = 0; b < 8; ++b)
            bits[i * 8 + b] = (val >> (7 - b)) & 1;
    }
}

/* Convert a bit array (MSB first) to a byte array */
void bitsToBytes(const int *bits, int bitCount, int *bytes)
{
    int byteCount = bitCount / 8;
    for (int i = 0; i < byteCount; ++i)
    {
        int val = 0;
        for (int b = 0; b < 8; ++b)
            val = (val << 1) | bits[i * 8 + b];
        bytes[i] = val;
    }
}

/* Generic permutation */
void permute(const int *in, int *out, const int *table, int n)
{
    for (int i = 0; i < n; ++i)
        out[i] = in[table[i] - 1];
}

/* Left rotate a 28‑bit half‑key */
void rotate28(int *half, int nShifts)
{
    int tmp[28];
    for (int i = 0; i < 28; ++i)
        tmp[i] = half[(i + nShifts) % 28];
    for (int i = 0; i < 28; ++i)
        half[i] = tmp[i];
}

/* Generate 16 sub‑keys (48 bits each) from a 64‑bit key */
void genSubKeys(const int *key64, int subKeys[16][48])
{
    int key56[56];
    permute(key64, key56, PC1, 56);

    int C[28], D[28];
    for (int i = 0; i < 28; ++i)
    {
        C[i] = key56[i];
        D[i] = key56[28 + i];
    }

    for (int round = 0; round < 16; ++round)
    {
        rotate28(C, SHIFTS[round]);
        rotate28(D, SHIFTS[round]);

        int CD[56];
        for (int i = 0; i < 28; ++i)
        {
            CD[i] = C[i];
            CD[28 + i] = D[i];
        }
        permute(CD, subKeys[round], PC2, 48);
    }
}

/* The DES f‑function */
void feistel(const int *R, const int *subKey, int *out)
{
    int expanded[48];
    permute(R, expanded, E, 48);
    for (int i = 0; i < 48; ++i)
        expanded[i] ^= subKey[i];

    int sOut[32];
    for (int s = 0; s < 8; ++s)
    {
        int row = (expanded[s * 6] << 1) | expanded[s * 6 + 5];
        int col = (expanded[s * 6 + 1] << 3) |
                  (expanded[s * 6 + 2] << 2) |
                  (expanded[s * 6 + 3] << 1) |
                  expanded[s * 6 + 4];
        int val = SBOX[s][row][col];
        for (int b = 0; b < 4; ++b)
            sOut[s * 4 + b] = (val >> (3 - b)) & 1;
    }
    permute(sOut, out, P, 32);
}

/* Single‑DES on a 64‑bit block */
void desBlock(const int *inBlock, int *outBlock, const int subKeys[16][48], bool encrypt)
{
    int permuted[64];
    permute(inBlock, permuted, IP, 64);

    int L[32], R[32];
    for (int i = 0; i < 32; ++i)
    {
        L[i] = permuted[i];
        R[i] = permuted[32 + i];
    }

    for (int round = 0; round < 16; ++round)
    {
        int nextR[32];
        feistel(R, subKeys[encrypt ? round : 15 - round], nextR);
        int newR[32];
        for (int i = 0; i < 32; ++i)
            newR[i] = L[i] ^ nextR[i];
        for (int i = 0; i < 32; ++i)
            L[i] = R[i];
        for (int i = 0; i < 32; ++i)
            R[i] = newR[i];
    }

    int preOut[64];
    for (int i = 0; i < 32; ++i)
        preOut[i] = R[i];
    for (int i = 0; i < 32; ++i)
        preOut[32 + i] = L[i];

    permute(preOut, outBlock, FP, 64);
}

/* ----------------------------------------------------------------- */
/*  3DES‑CBC encryption (encrypt = true)                             */
/* ----------------------------------------------------------------- */
void tripleDesCbcEncrypt(const int *plainBits, int plainBitsLen,
                         const int *k1, const int *k2, const int *k3,
                         const int *ivBits,
                         int *cipherBits)
{
    int sub1[16][48], sub2[16][48], sub3[16][48];
    genSubKeys(k1, sub1);
    genSubKeys(k2, sub2);
    genSubKeys(k3, sub3);

    int blockPrev[64];
    copyBits(ivBits, blockPrev, 64);

    int blockIn[64], blockTmp[64], blockOut[64];
    int blocks = plainBitsLen / 64;
    for (int b = 0; b < blocks; ++b)
    {
        /* XOR with previous ciphertext (CBC) */
        for (int i = 0; i < 64; ++i)
            blockIn[i] = plainBits[b * 64 + i] ^ blockPrev[i];

        /* 3DES: E(K1) – D(K2) – E(K3) */
        desBlock(blockIn, blockTmp, sub1, true);
        desBlock(blockTmp, blockTmp, sub2, false);
        desBlock(blockTmp, blockOut, sub3, true);

        copyBits(blockOut, &cipherBits[b * 64], 64);
        copyBits(blockOut, blockPrev, 64);
    }
}

/* ----------------------------------------------------------------- */
/*  PKCS#5 padding for a byte array (max block count = 8)            */
/* ----------------------------------------------------------------- */
int padPkcs5(const int *inBytes, int inLen, int *outBytes)
{
    int pad = 8 - (inLen % 8);
    for (int i = 0; i < inLen; ++i) outBytes[i] = inBytes[i];
    for (int i = 0; i < pad; ++i) outBytes[inLen + i] = pad;
    return inLen + pad;
}

/* ----------------------------------------------------------------- */
/*  Print a byte array as hex                                        */
/* ----------------------------------------------------------------- */
void printHex(const int *bytes, int count)
{
    for (int i = 0; i < count; ++i)
        std::cout << std::hex << std::setw(2) << std::setfill('0') << bytes[i];
    std::cout << std::dec << std::endl;
}

/* ----------------------------------------------------------------- */
/*  Main – generate edge‑case inputs and run 3DES‑CBC                */
/* ----------------------------------------------------------------- */
int main()
{
    /* -----------------------------------------------------------------
       Edge‑case test vectors (all generated internally)
       1) Empty plaintext
       2) One block of all 0xFF
       3) One block of all zeros
       4) Two blocks, reversed pattern (0x55 then 0xAA)
       5) Large input (8 blocks) of descending bytes
       6) Near‑max input (63 bytes) of descending bytes
       ----------------------------------------------------------------- */

    int testCnt = 6;
    for (int t = 0; t < testCnt; ++t)
    {
        int plainBytes[64];          // max 8 blocks => 64 bytes
        int plainLen = 0;

        if (t == 0)                 // empty
        {
            plainLen = 0;
        }
        else if (t == 1)            // all 0xFF, 1 block
        {
            for (int i = 0; i < 8; ++i) plainBytes[i] = 255;
            plainLen = 8;
        }
        else if (t == 2)            // all zeros, 1 block
        {
            for (int i = 0; i < 8; ++i) plainBytes[i] = 0;
            plainLen = 8;
        }
        else if (t == 3)            // two blocks, 0x55 then 0xAA
        {
            for (int i = 0; i < 8; ++i) plainBytes[i] = 0x55;
            for (int i = 8; i < 16; ++i) plainBytes[i] = 0xAA;
            plainLen = 16;
        }
        else if (t == 4)            // large 8‑block descending data
        {
            for (int i = 0; i < 64; ++i) plainBytes[i] = 255 - i;
            plainLen = 64;
        }
        else                         // 63‑byte descending data (worst‑case padding)
        {
            for (int i = 0; i < 63; ++i) plainBytes[i] = 63 - i;
            plainLen = 63;
        }

        /* ----- PKCS#5 padding (required for CBC) ----- */
        int paddedBytes[72];                 // enough for padding
        int paddedLen = padPkcs5(plainBytes, plainLen, paddedBytes);

        /* ----- Convert to bit array ----- */
        int plainBits[576];                  // 9 blocks * 64 bits max
        bytesToBits(paddedBytes, paddedLen, plainBits);
        int plainBitsLen = paddedLen * 8;

        /* ----- Fixed 3DES keys (64‑bit each, parity bits ignored) ----- */
        int key1Bytes[8] = {0x13,0x34,0x57,0x79,0x9B,0xBC,0xDF,0xF1};
        int key2Bytes[8] = {0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF};
        int key3Bytes[8] = {0xFE,0xDC,0xBA,0x98,0x76,0x54,0x32,0x10};

        int key1Bits[64], key2Bits[64], key3Bits[64];
        bytesToBits(key1Bytes, 8, key1Bits);
        bytesToBits(key2Bytes, 8, key2Bits);
        bytesToBits(key3Bytes, 8, key3Bits);

        /* ----- IV (all zeros) ----- */
        int ivBits[64];
        for (int i = 0; i < 64; ++i) ivBits[i] = 0;

        /* ----- Encrypt ----- */
        int cipherBits[576];
        tripleDesCbcEncrypt(plainBits, plainBitsLen,
                            key1Bits, key2Bits, key3Bits,
                            ivBits, cipherBits);

        /* ----- Convert ciphertext back to bytes ----- */
        int cipherBytes[72];
        bitsToBytes(cipherBits, plainBitsLen, cipherBytes);
        int cipherByteCount = plainBitsLen / 8;

        /* ----- Output ----- */
        std::cout << "Test " << t << " (plain bytes = " << plainLen
                  << ", padded = " << paddedLen << "): ";
        printHex(cipherBytes, cipherByteCount);
    }
    return 0;
}
