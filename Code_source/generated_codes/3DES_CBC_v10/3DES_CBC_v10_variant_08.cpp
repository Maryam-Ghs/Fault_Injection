/* LLM input variant 8: sparse-skewed */
// 3DES-CBC implementation – version #10
// ------------------------------------------------
// Only int is used for data representation.
// All data structures are allocated on the heap.
// Small predefined plaintext, keys and IV are used.
// ------------------------------------------------

#include <iostream>
#include <iomanip>

// ---------------------------------------------------------------------
//  Permutation helper (generic)
// ---------------------------------------------------------------------
void permuteBits(const int* src, int* dst, const int* table, int size)
{
    int i = 0;
    while (i < size)
    {
        dst[i] = src[table[i] - 1];
        ++i;
    }
}

// ---------------------------------------------------------------------
//  Left‑circular shift for a half‑key (28 bits)
// ---------------------------------------------------------------------
void rotateHalf(int* half, int steps)
{
    int i = 0;
    while (i < steps)
    {
        int first = half[0];
        int j = 0;
        while (j < 27)
        {
            half[j] = half[j + 1];
            ++j;
        }
        half[27] = first;
        ++i;
    }
}

// ---------------------------------------------------------------------
//  Generate 16 sub‑keys (48 bits each) from a 64‑bit key
// ---------------------------------------------------------------------
void genSubKeys(const int* keyBytes, int*** subKeys)
{
    // Tables (non‑const, int only)
    int ipt[56] = {
        57,49,41,33,25,17,9,
        1,58,50,42,34,26,18,
        10,2,59,51,43,35,27,
        19,11,3,60,52,44,36,
        63,55,47,39,31,23,15,
        7,62,54,46,38,30,22,
        14,6,61,53,45,37,29,
        21,13,5,28,20,12,4
    };
    int p2t[48] = {
        14,17,11,24,1,5,
        3,28,15,6,21,10,
        23,19,12,4,26,8,
        16,7,27,20,13,2,
        41,52,31,37,47,55,
        30,40,51,45,33,48,
        44,49,39,56,34,53,
        46,42,50,36,29,32
    };
    int shiftTab[16] = {
        1,1,2,2,2,2,2,2,
        1,2,2,2,2,2,2,1
    };

    // ----- 1. Convert key bytes to bits (64) -----
    int* keyBits = new int[64];
    int b = 0;
    while (b < 8)
    {
        int bit = 0;
        while (bit < 8)
        {
            keyBits[b * 8 + bit] = (keyBytes[b] >> (7 - bit)) & 1;
            ++bit;
        }
        ++b;
    }

    // ----- 2. Apply PC‑1 permutation (56 bits) -----
    int* permKey = new int[56];
    permuteBits(keyBits, permKey, ipt, 56);
    delete[] keyBits;

    // ----- 3. Split into C and D (28 bits each) -----
    int* left = new int[28];
    int* right = new int[28];
    int i = 0;
    while (i < 28)
    {
        left[i] = permKey[i];
        right[i] = permKey[i + 28];
        ++i;
    }
    delete[] permKey;

    // ----- 4. Produce 16 sub‑keys -----
    int round = 0;
    while (round < 16)
    {
        rotateHalf(left, shiftTab[round]);
        rotateHalf(right, shiftTab[round]);

        // combine C and D (56 bits)
        int* combined = new int[56];
        i = 0;
        while (i < 28)
        {
            combined[i] = left[i];
            combined[i + 28] = right[i];
            ++i;
        }

        // permute with PC‑2 -> sub‑key (48 bits)
        int* sub = new int[48];
        permuteBits(combined, sub, p2t, 48);
        delete[] combined;

        // store pointer
        (*subKeys)[round] = sub;
        ++round;
    }

    delete[] left;
    delete[] right;
}

// ---------------------------------------------------------------------
//  The DES round function F
// ---------------------------------------------------------------------
void feistelF(const int* halfR, const int* subKey, int* out32)
{
    // Expansion table (E) – 48 entries
    int eTab[48] = {
        32,1,2,3,4,5,
        4,5,6,7,8,9,
        8,9,10,11,12,13,
        12,13,14,15,16,17,
        16,17,18,19,20,21,
        20,21,22,23,24,25,
        24,25,26,27,28,29,
        28,29,30,31,32,1
    };
    // S‑boxes (8 × 4 × 16)
    int sBox[8][4][16] = {
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
    // Permutation P (32)
    int pTab[32] = {
        16,7,20,21,
        29,12,28,17,
        1,15,23,26,
        5,18,31,10,
        2,8,24,14,
        32,27,3,9,
        19,13,30,6,
        22,11,4,25
    };

    // ----- Expand R to 48 bits -----
    int* expR = new int[48];
    int i = 0;
    while (i < 48)
    {
        expR[i] = halfR[eTab[i] - 1];
        ++i;
    }

    // ----- XOR with sub‑key -----
    i = 0;
    while (i < 48)
    {
        expR[i] ^= subKey[i];
        ++i;
    }

    // ----- S‑box substitution (8 × 6 → 4) -----
    int* sOut = new int[32];
    int sIdx = 0;
    while (sIdx < 8)
    {
        int row = (expR[sIdx * 6] << 1) | expR[sIdx * 6 + 5];
        int col = (expR[sIdx * 6 + 1] << 3) |
                  (expR[sIdx * 6 + 2] << 2) |
                  (expR[sIdx * 6 + 3] << 1) |
                  expR[sIdx * 6 + 4];
        int val = sBox[sIdx][row][col];

        // split the 4‑bit value into bits
        int b = 0;
        while (b < 4)
        {
            sOut[sIdx * 4 + (3 - b)] = (val >> b) & 1;
            ++b;
        }
        ++sIdx;
    }

    delete[] expR;

    // ----- Permutation P -----
    i = 0;
    while (i < 32)
    {
        out32[i] = sOut[pTab[i] - 1];
        ++i;
    }
    delete[] sOut;
}

// ---------------------------------------------------------------------
//  Single‑DES block (64‑bit) – encrypt if enc==true, else decrypt
// ---------------------------------------------------------------------
void desBlock(const int* inBlk, int* outBlk,
              int** subKeys, bool enc)
{
    // Fixed tables (int only)
    int ipTab[64] = {
        58,50,42,34,26,18,10,2,
        60,52,44,36,28,20,12,4,
        62,54,46,38,30,22,14,6,
        64,56,48,40,32,24,16,8,
        57,49,41,33,25,17,9,1,
        59,51,43,35,27,19,11,3,
        61,53,45,37,29,21,13,5,
        63,55,47,39,31,23,15,7
    };
    int fpTab[64] = {
        40,8,48,16,56,24,64,32,
        39,7,47,15,55,23,63,31,
        38,6,46,14,54,22,62,30,
        37,5,45,13,53,21,61,29,
        36,4,44,12,52,20,60,28,
        35,3,43,11,51,19,59,27,
        34,2,42,10,50,18,58,26,
        33,1,41,9,49,17,57,25
    };

    // ----- Convert bytes to bits -----
    int* bits = new int[64];
    int b = 0;
    while (b < 8)
    {
        int bit = 0;
        while (bit < 8)
        {
            bits[b * 8 + bit] = (inBlk[b] >> (7 - bit)) & 1;
            ++bit;
        }
        ++b;
    }

    // ----- Initial permutation -----
    int* ipBits = new int[64];
    permuteBits(bits, ipBits, ipTab, 64);
    delete[] bits;

    // ----- Split L and R (32 each) -----
    int* left = new int[32];
    int* right = new int[32];
    int i = 0;
    while (i < 32)
    {
        left[i] = ipBits[i];
        right[i] = ipBits[i + 32];
        ++i;
    }
    delete[] ipBits;

    // ----- 16 rounds -----
    int round = 0;
    while (round < 16)
    {
        int* fOut = new int[32];
        int* sub = enc ? subKeys[round] : subKeys[15 - round];
        feistelF(right, sub, fOut);

        // new left = right
        int* newLeft = new int[32];
        int j = 0;
        while (j < 32)
        {
            newLeft[j] = right[j];
            ++j;
        }

        // new right = left XOR fOut
        int* newRight = new int[32];
        j = 0;
        while (j < 32)
        {
            newRight[j] = left[j] ^ fOut[j];
            ++j;
        }

        delete[] left;
        delete[] right;
        delete[] fOut;

        left = newLeft;
        right = newRight;
        ++round;
    }

    // ----- Combine (note the swap) -----
    int* preOut = new int[64];
    i = 0;
    while (i < 32)
    {
        preOut[i] = right[i];
        preOut[i + 32] = left[i];
        ++i;
    }
    delete[] left;
    delete[] right;

    // ----- Final permutation -----
    int* fpBits = new int[64];
    permuteBits(preOut, fpBits, fpTab, 64);
    delete[] preOut;

    // ----- Convert bits back to bytes -----
    i = 0;
    while (i < 8)
    {
        int val = 0;
        int bit = 0;
        while (bit < 8)
        {
            val = (val << 1) | fpBits[i * 8 + bit];
            ++bit;
        }
        outBlk[i] = val;
        ++i;
    }
    delete[] fpBits;
}

// ---------------------------------------------------------------------
//  Triple‑DES (EDE) on a single block
// ---------------------------------------------------------------------
void tripleDesBlock(const int* inBlk, int* outBlk,
                    int*** k1, int*** k2, int*** k3, bool enc)
{
    int* tmp = new int[8];
    // 1st DES
    desBlock(inBlk, tmp, *k1, true);
    // 2nd DES (decrypt)
    desBlock(tmp, outBlk, *k2, false);
    // 3rd DES
    desBlock(outBlk, tmp, *k3, true);
    // copy final result
    int i = 0;
    while (i < 8)
    {
        outBlk[i] = tmp[i];
        ++i;
    }
    delete[] tmp;
}

// ---------------------------------------------------------------------
//  CBC mode encryption for a small message (multiple of 8 bytes)
// ---------------------------------------------------------------------
void cbcEncrypt(const int* plain, int blkCnt,
                int*** sub1, int*** sub2, int*** sub3,
                const int* iv, int* cipher)
{
    // previous ciphertext block (starts with IV)
    int* prev = new int[8];
    int i = 0;
    while (i < 8)
    {
        prev[i] = iv[i];
        ++i;
    }

    int blk = 0;
    while (blk < blkCnt)
    {
        // XOR with previous ciphertext (or IV)
        int* xored = new int[8];
        i = 0;
        while (i < 8)
        {
            xored[i] = plain[blk * 8 + i] ^ prev[i];
            ++i;
        }

        // Triple‑DES encrypt
        int* outBlk = new int[8];
        tripleDesBlock(xored, outBlk, sub1, sub2, sub3, true);

        // store ciphertext
        i = 0;
        while (i < 8)
        {
            cipher[blk * 8 + i] = outBlk[i];
            ++i;
        }

        // next IV = current ciphertext
        delete[] prev;
        prev = outBlk;

        delete[] xored;
        ++blk;
    }
    delete[] prev;
}

// ---------------------------------------------------------------------
//  Helper to print a byte array as hex
// ---------------------------------------------------------------------
void printHex(const int* data, int size)
{
    int i = 0;
    while (i < size)
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (data[i] & 0xff);
        if ((i + 1) % 8 == 0) std::cout << " ";
        ++i;
    }
    std::cout << std::dec << std::endl;
}

// ---------------------------------------------------------------------
//  Main – generate data, run CBC, show result
// ---------------------------------------------------------------------
int main()
{
    // ---- Sparse predefined plaintext (2 blocks) ----
    int* message = new int[16]{
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02
    };

    // ---- Sparse three 8‑byte keys ----
    int* keyA = new int[8]{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01};
    int* keyB = new int[8]{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02};
    int* keyC = new int[8]{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03};

    // ---- IV (mostly zeros, one non‑zero byte) ----
    int* iv = new int[8]{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF};

    // ---- Allocate sub‑key containers (16 × 48 bits) ----
    int*** subA = new int**[1];
    subA[0] = new int*[16];
    int*** subB = new int**[1];
    subB[0] = new int*[16];
    int*** subC = new int**[1];
    subC[0] = new int*[16];

    // ---- Generate sub‑keys for each key ----
    genSubKeys(keyA, subA);
    genSubKeys(keyB, subB);
    genSubKeys(keyC, subC);

    // ---- Prepare ciphertext buffer ----
    int blockCount = 2;               // 16 bytes / 8 = 2 blocks
    int* cipher = new int[blockCount * 8];

    // ---- Encrypt in CBC mode ----
    cbcEncrypt(message, blockCount, subA, subB, subC, iv, cipher);

    // ---- Output ----
    std::cout << "Plaintext : ";
    printHex(message, blockCount * 8);
    std::cout << "Ciphertext: ";
    printHex(cipher, blockCount * 8);

    // ---- Clean up ----
    delete[] message;
    delete[] keyA;
    delete[] keyB;
    delete[] keyC;
    delete[] iv;
    delete[] cipher;

    // sub‑keys
    int r = 0;
    while (r < 16)
    {
        delete[] subA[0][r];
        delete[] subB[0][r];
        delete[] subC[0][r];
        ++r;
    }
    delete[] subA[0];
    delete[] subB[0];
    delete[] subC[0];
    delete[] subA;
    delete[] subB;
    delete[] subC;

    return 0;
}
