/* LLM input variant 9: medium-deterministic-random */
#include <iostream>
#include <iomanip>

//--- permutation tables -------------------------------------------------
static int IP[64] = {
    58,50,42,34,26,18,10,2, 60,52,44,36,28,20,12,4,
    62,54,46,38,30,22,14,6, 64,56,48,40,32,24,16,8,
    57,49,41,33,25,17,9,1, 59,51,43,35,27,19,11,3,
    61,53,45,37,29,21,13,5, 63,55,47,39,31,23,15,7
};

static int FP[64] = {
    40,8,48,16,56,24,64,32, 39,7,47,15,55,23,63,31,
    38,6,46,14,54,22,62,30, 37,5,45,13,53,21,61,29,
    36,4,44,12,52,20,60,28, 35,3,43,11,51,19,59,27,
    34,2,42,10,50,18,58,26, 33,1,41,9,49,17,57,25
};

static int E[48] = {
    32,1,2,3,4,5, 4,5,6,7,8,9, 8,9,10,11,12,13,
    12,13,14,15,16,17, 16,17,18,19,20,21, 20,21,22,23,24,25,
    24,25,26,27,28,29, 28,29,30,31,32,1
};

static int P[32] = {
    16,7,20,21, 29,12,28,17, 1,15,23,26, 5,18,31,10,
    2,8,24,14, 32,27,3,9, 19,13,30,6, 22,11,4,25
};

static int PC1[56] = {
    57,49,41,33,25,17,9, 1,58,50,42,34,26,18,
    10,2,59,51,43,35,27,19,11,3,60,52,44,36,
    63,55,47,39,31,23,15,7, 62,54,46,38,30,22,
    14,6,61,53,45,37,29,21,13,5,28,20,12,4
};

static int PC2[48] = {
    14,17,11,24,1,5, 3,28,15,6,21,10,
    23,19,12,4,26,8, 16,7,27,20,13,2,
    41,52,31,37,47,55, 30,40,51,45,33,48,
    44,49,39,56,34,53, 46,42,50,36,29,32
};

static int SHIFTS[16] = {
    1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1
};

//--- S‑boxes ------------------------------------------------------------
static int SBOX[8][4][16] = {
    {
        {14,4,13,1,2,15,11,8,3,10,6,12,5,9,0,7},
        {0,15,7,4,14,2,13,1,10,6,12,11,9,5,3,8},
        {4,1,14,8,13,6,2,11,15,12,9,7,3,10,5,0},
        {15,12,8,2,4,9,1,7,5,11,3,14,10,0,6,13}
    },
    {
        {15,1,8,14,6,11,3,4,9,7,2,13,12,0,5,10},
        {3,13,4,7,15,2,8,14,12,0,1,10,6,9,11,5},
        {0,14,7,11,10,4,13,1,5,8,12,6,9,3,2,15},
        {13,8,10,1,3,15,4,2,11,6,7,12,0,5,14,9}
    },
    {
        {10,0,9,14,6,3,15,5,1,13,12,7,11,4,2,8},
        {13,7,0,9,3,4,6,10,2,8,5,14,12,11,15,1},
        {13,6,4,9,8,15,3,0,11,1,2,12,5,10,14,7},
        {1,10,13,0,6,9,8,7,4,15,14,3,11,5,2,12}
    },
    {
        {7,13,14,3,0,6,9,10,1,2,8,5,11,12,4,15},
        {13,8,11,5,6,15,0,3,4,7,2,12,1,10,14,9},
        {10,6,9,0,12,11,7,13,15,1,3,14,5,2,8,4},
        {3,15,0,6,10,1,13,8,9,4,5,11,12,7,2,14}
    },
    {
        {2,12,4,1,7,10,11,6,8,5,3,15,13,0,14,9},
        {14,11,2,12,4,7,13,1,5,0,15,10,3,9,8,6},
        {4,2,1,11,10,13,7,8,15,9,12,5,6,3,0,14},
        {11,8,12,7,1,14,2,13,6,15,0,9,10,4,5,3}
    },
    {
        {12,1,10,15,9,2,6,8,0,13,3,4,14,7,5,11},
        {10,15,4,2,7,12,9,5,6,1,13,14,0,11,3,8},
        {9,14,15,5,2,8,12,3,7,0,4,10,1,13,11,6},
        {4,3,2,12,9,5,15,10,11,14,1,7,6,0,8,13}
    },
    {
        {4,11,2,14,15,0,8,13,3,12,9,7,5,10,6,1},
        {13,0,11,7,4,9,1,10,14,3,5,12,2,15,8,6},
        {1,4,11,13,12,3,7,14,10,15,6,8,0,5,9,2},
        {6,11,13,8,1,4,10,7,9,5,0,15,14,2,3,12}
    }
};

//--- utility ------------------------------------------------------------
void bytesToBits(const int src[8], int dst[64])
{
    int i = 0, j = 0;
    while (i < 8) {
        int v = src[i];
        j = 0;
        while (j < 8) {
            dst[i*8 + j] = (v >> (7 - j)) & 1;
            ++j;
        }
        ++i;
    }
}

void bitsToBytes(const int src[64], int dst[8])
{
    int i = 0, j = 0;
    while (i < 8) {
        int v = 0;
        j = 0;
        while (j < 8) {
            v = (v << 1) | src[i*8 + j];
            ++j;
        }
        dst[i] = v;
        ++i;
    }
}

void permute(const int src[], int dst[], const int table[], int n)
{
    int i = 0;
    while (i < n) {
        dst[i] = src[table[i] - 1];
        ++i;
    }
}

//--- key schedule -------------------------------------------------------
void makeSubKeys(const int key[8], int sub[16][48])
{
    int keyBits[64];
    bytesToBits(key, keyBits);

    int pc1Bits[56];
    permute(keyBits, pc1Bits, PC1, 56);

    int C[28], D[28];
    int i = 0;
    while (i < 28) { C[i] = pc1Bits[i]; D[i] = pc1Bits[i+28]; ++i; }

    int round = 0;
    while (round < 16) {
        int shift = SHIFTS[round];
        int s = 0;
        while (s < shift) {
            // left rotate C
            int tmpC = C[0];
            int t = 0;
            while (t < 27) { C[t] = C[t+1]; ++t; }
            C[27] = tmpC;
            // left rotate D
            int tmpD = D[0];
            t = 0;
            while (t < 27) { D[t] = D[t+1]; ++t; }
            D[27] = tmpD;
            ++s;
        }

        int combined[56];
        i = 0;
        while (i < 28) { combined[i] = C[i]; ++i; }
        i = 0;
        while (i < 28) { combined[28+i] = D[i]; ++i; }

        permute(combined, sub[round], PC2, 48);
        ++round;
    }
}

//--- f‑function ---------------------------------------------------------
int fFunction(int r, const int sub[48])
{
    int rBits[32];
    int i = 0;
    while (i < 32) { rBits[i] = (r >> (31 - i)) & 1; ++i; }

    int eBits[48];
    permute(rBits, eBits, E, 48);

    i = 0;
    while (i < 48) { eBits[i] ^= sub[i]; ++i; }

    int sOut = 0;
    int block = 0;
    while (block < 8) {
        int six = 0;
        i = 0;
        while (i < 6) { six = (six << 1) | eBits[block*6 + i]; ++i; }
        int row = ((six & 0x20) >> 4) | (six & 0x01);
        int col = (six >> 1) & 0x0F;
        int val = SBOX[block][row][col];
        sOut = (sOut << 4) | val;
        ++block;
    }

    int sBits[32];
    i = 0;
    while (i < 32) { sBits[i] = (sOut >> (31 - i)) & 1; ++i; }

    int pBits[32];
    permute(sBits, pBits, P, 32);

    int res = 0;
    i = 0;
    while (i < 32) { res = (res << 1) | pBits[i]; ++i; }
    return res;
}

//--- single DES ---------------------------------------------------------
void desBlock(const int in[8], int out[8], const int key[8], bool encrypt)
{
    int subKeys[16][48];
    makeSubKeys(key, subKeys);

    int ipBits[64];
    int ipOut[64];
    int blockBits[64];
    bytesToBits(in, blockBits);
    permute(blockBits, ipBits, IP, 64);

    int L = 0, R = 0;
    int i = 0;
    while (i < 32) { L = (L << 1) | ipBits[i]; ++i; }
    i = 32;
    while (i < 64) { R = (R << 1) | ipBits[i]; ++i; }

    int round = 0;
    while (round < 16) {
        int subIdx = encrypt ? round : 15 - round;
        int temp = R;
        R = L ^ fFunction(R, subKeys[subIdx]);
        L = temp;
        ++round;
    }

    // preoutput (swap)
    int pre[64];
    i = 0;
    while (i < 32) { pre[i] = (R >> (31 - i)) & 1; ++i; }
    i = 0;
    while (i < 32) { pre[32 + i] = (L >> (31 - i)) & 1; ++i; }

    permute(pre, ipOut, FP, 64);
    bitsToBytes(ipOut, out);
}

//--- 3DES‑EDE ------------------------------------------------------------
void tripleDESEDE(const int pt[8], int ct[8],
                  const int k1[8], const int k2[8], const int k3[8])
{
    int step1[8], step2[8];
    desBlock(pt, step1, k1, true);      // encrypt with K1
    desBlock(step1, step2, k2, false); // decrypt with K2
    desBlock(step2, ct, k3, true);      // encrypt with K3
}

//--- main ---------------------------------------------------------------
int main()
{
    // plaintext 0xFEDCBA9876543210
    int plain[8] = {0xFE,0xDC,0xBA,0x98,0x76,0x54,0x32,0x10};

    // three keys (deterministic pseudo‑random values)
    int key1[8] = {0x0F,0x1E,0x2D,0x3C,0x4B,0x5A,0x69,0x78};
    int key2[8] = {0x89,0x9A,0xAB,0xBC,0xCD,0xDE,0xEF,0xF0};
    int key3[8] = {0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF};

    int cipher[8];
    tripleDESEDE(plain, cipher, key1, key2, key3);

    std::cout << "Plaintext : ";
    int i = 0;
    while (i < 8) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << plain[i];
        ++i;
    }
    std::cout << std::dec << "\nCiphertext: ";
    i = 0;
    while (i < 8) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << cipher[i];
        ++i;
    }
    std::cout << std::dec << std::endl;
    return 0;
}
