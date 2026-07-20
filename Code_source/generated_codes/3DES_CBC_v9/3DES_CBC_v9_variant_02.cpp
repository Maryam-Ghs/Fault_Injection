/* LLM input variant 2: small-diverse */
#include <iostream>
#include <iomanip>

/* -------------------------------------------------------------
   Tables – all are plain int arrays (no const, no unsigned)
   ------------------------------------------------------------- */
int IP[64] = {
     58, 50, 42, 34, 26, 18, 10,  2,
     60, 52, 44, 36, 28, 20, 12,  4,
     62, 54, 46, 38, 30, 22, 14,  6,
     64, 56, 48, 40, 32, 24, 16,  8,
     57, 49, 41, 33, 25, 17,  9,  1,
     59, 51, 43, 35, 27, 19, 11,  3,
     61, 53, 45, 37, 29, 21, 13,  5,
     63, 55, 47, 39, 31, 23, 15,  7};

int FP[64] = {
     40,  8, 48, 16, 56, 24, 64, 32,
     39,  7, 47, 15, 55, 23, 63, 31,
     38,  6, 46, 14, 54, 22, 62, 30,
     37,  5, 45, 13, 53, 21, 61, 29,
     36,  4, 44, 12, 52, 20, 60, 28,
     35,  3, 43, 11, 51, 19, 59, 27,
     34,  2, 42, 10, 50, 18, 58, 26,
     33,  1, 41,  9, 49, 17, 57, 25};

int EBOX[48] = {
     32,  1,  2,  3,  4,  5,
      4,  5,  6,  7,  8,  9,
      8,  9, 10, 11, 12, 13,
     12, 13, 14, 15, 16, 17,
     16, 17, 18, 19, 20, 21,
     20, 21, 22, 23, 24, 25,
     24, 25, 26, 27, 28, 29,
     28, 29, 30, 31, 32,  1};

int PBOX[32] = {
     16,  7, 20, 21,
     29, 12, 28, 17,
      1, 15, 23, 26,
      5, 18, 31, 10,
      2,  8, 24, 14,
     32, 27,  3,  9,
     19, 13, 30,  6,
     22, 11,  4, 25};

int PC1[56] = {
     57, 49, 41, 33, 25, 17,  9,
      1, 58, 50, 42, 34, 26, 18,
     10,  2, 59, 51, 43, 35, 27,
     19, 11,  3, 60, 52, 44, 36,
     63, 55, 47, 39, 31, 23, 15,
      7, 62, 54, 46, 38, 30, 22,
     14,  6, 61, 53, 45, 37, 29,
     21, 13,  5, 28, 20, 12,  4};

int PC2[48] = {
     14, 17, 11, 24,  1,  5,
      3, 28, 15,  6, 21, 10,
     23, 19, 12,  4, 26,  8,
     16,  7, 27, 20, 13,  2,
     41, 52, 31, 37, 47, 55,
     30, 40, 51, 45, 33, 48,
     44, 49, 39, 56, 34, 53,
     46, 42, 50, 36, 29, 32};

int SHIFTS[16] = {
     1, 1, 2, 2, 2, 2, 2, 2,
     1, 2, 2, 2, 2, 2, 2, 1};

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

/* -------------------------------------------------------------
   Helper: bit extraction (1‑based position)
   ------------------------------------------------------------- */
int getBit(int val, int pos) {
    return (val >> (32 - pos)) & 1;
}

/* -------------------------------------------------------------
   Class – Triple DES in CBC mode
   ------------------------------------------------------------- */
class TripleDES_CBC {
public:
    void setKeySet(const int k1[8], const int k2[8], const int k3[8]) {
        genSubKeys(k1, subK1);
        genSubKeys(k2, subK2);
        genSubKeys(k3, subK3);
    }

    void encrypt(const int *plain, int *cipher, int blocks, const int iv[8]) {
        int prevL = 0, prevR = 0;
        // load IV (big‑endian)
        for (int i = 0; i < 4; ++i) {
            prevL = (prevL << 8) | (iv[i] & 0xFF);
            prevR = (prevR << 8) | (iv[i + 4] & 0xFF);
        }

        for (int b = 0; b < blocks; ++b) {
            int curL = 0, curR = 0;
            // load block
            for (int i = 0; i < 4; ++i) {
                curL = (curL << 8) | (plain[b * 8 + i] & 0xFF);
                curR = (curR << 8) | (plain[b * 8 + i + 4] & 0xFF);
            }
            // CBC XOR
            curL ^= prevL; curR ^= prevR;

            // 3DES (EDE)
            desRound(curL, curR, subK1, 1);   // encrypt with K1
            desRound(curL, curR, subK2, 0);   // decrypt with K2
            desRound(curL, curR, subK3, 1);   // encrypt with K3

            // store ciphertext
            for (int i = 3; i >= 0; --i) {
                cipher[b * 8 + i] = curR & 0xFF; curR >>= 8;
                cipher[b * 8 + i + 4] = curL & 0xFF; curL >>= 8;
            }
            // update chaining value
            prevL = 0; prevR = 0;
            for (int i = 0; i < 4; ++i) {
                prevL = (prevL << 8) | (cipher[b * 8 + i] & 0xFF);
                prevR = (prevR << 8) | (cipher[b * 8 + i + 4] & 0xFF);
            }
        }
    }

private:
    int subK1[16][2];
    int subK2[16][2];
    int subK3[16][2];

    /* ---------------------------------------------------------
       Generate 16 sub‑keys (each 48‑bit stored as two 24‑bit ints)
       --------------------------------------------------------- */
    void genSubKeys(const int key[8], int sub[16][2]) {
        int c = 0, d = 0;
        // PC‑1 permutation (56 bits)
        for (int i = 0; i < 28; ++i) {
            int bit = (key[(PC1[i] - 1) / 8] >> (7 - ((PC1[i] - 1) % 8))) & 1;
            c = (c << 1) | bit;
        }
        for (int i = 28; i < 56; ++i) {
            int bit = (key[(PC1[i] - 1) / 8] >> (7 - ((PC1[i] - 1) % 8))) & 1;
            d = (d << 1) | bit;
        }

        // 16 rounds
        for (int r = 0; r < 16; ++r) {
            // left rotations
            int rot = SHIFTS[r];
            c = ((c << rot) | (c >> (28 - rot))) & 0x0FFFFFFF;
            d = ((d << rot) | (d >> (28 - rot))) & 0x0FFFFFFF;

            // PC‑2 compression (48 bits)
            int left = 0, right = 0;
            for (int i = 0; i < 24; ++i) {
                int src = (PC2[i] <= 28) ? c : d;
                int pos = (PC2[i] - 1) % 28 + 1;
                int bit = (src >> (28 - pos)) & 1;
                left = (left << 1) | bit;
            }
            for (int i = 24; i < 48; ++i) {
                int src = (PC2[i] <= 28) ? c : d;
                int pos = (PC2[i] - 1) % 28 + 1;
                int bit = (src >> (28 - pos)) & 1;
                right = (right << 1) | bit;
            }
            sub[r][0] = left;   // 24‑bit part
            sub[r][1] = right;  // 24‑bit part
        }
    }

    /* ---------------------------------------------------------
       Single DES round set (encrypt/decrypt flag)
       --------------------------------------------------------- */
    void desRound(int &L, int &R, const int sub[16][2], int enc) {
        int tempL = L, tempR = R;
        // Initial permutation
        permuteIP(tempL, tempR);
        // 16 Feistel rounds
        for (int i = 0; i < 16; ++i) {
            int round = enc ? i : 15 - i;
            int fout = fFunction(tempR, sub[round]);
            int newL = tempL ^ fout;
            // swap without extra branch
            tempL = tempR;
            tempR = newL;
        }
        // Undo final swap
        L = tempR; R = tempL;
        // Final permutation
        permuteFP(L, R);
    }

    /* ---------------------------------------------------------
       Initial permutation (IP)
       --------------------------------------------------------- */
    void permuteIP(int &L, int &R) {
        int in[2] = { L, R };
        int outL = 0, outR = 0;
        for (int i = 0; i < 32; ++i) {
            int src = IP[i] - 1;
            int bit = (in[src / 32] >> (31 - (src % 32))) & 1;
            outL = (outL << 1) | bit;
        }
        for (int i = 32; i < 64; ++i) {
            int src = IP[i] - 1;
            int bit = (in[src / 32] >> (31 - (src % 32))) & 1;
            outR = (outR << 1) | bit;
        }
        L = outL; R = outR;
    }

    /* ---------------------------------------------------------
       Final permutation (FP)
       --------------------------------------------------------- */
    void permuteFP(int &L, int &R) {
        int in[2] = { L, R };
        int outL = 0, outR = 0;
        for (int i = 0; i < 32; ++i) {
            int src = FP[i] - 1;
            int bit = (in[src / 32] >> (31 - (src % 32))) & 1;
            outL = (outL << 1) | bit;
        }
        for (int i = 32; i < 64; ++i) {
            int src = FP[i] - 1;
            int bit = (in[src / 32] >> (31 - (src % 32))) & 1;
            outR = (outR << 1) | bit;
        }
        L = outL; R = outR;
    }

    /* ---------------------------------------------------------
       Feistel function F (32‑bit R, 48‑bit sub‑key)
       --------------------------------------------------------- */
    int fFunction(int R, const int sub[2]) {
        int expanded = 0;
        // Expansion E (32 → 48)
        for (int i = 0; i < 48; ++i) {
            int src = EBOX[i] - 1;
            int bit = (R >> (31 - src)) & 1;
            expanded = (expanded << 1) | bit;
        }
        // XOR with sub‑key (both halves are 24‑bit)
        expanded ^= (sub[0] << 24) | sub[1];

        // S‑box substitution (8 groups of 6 bits)
        int sOut = 0;
        for (int i = 0; i < 8; ++i) {
            int six = (expanded >> (42 - 6 * i)) & 0x3F;
            int row = ((six >> 5) << 1) | (six & 1);
            int col = (six >> 1) & 0xF;
            int val = SBOX[i][row][col];
            sOut = (sOut << 4) | val;
        }

        // Permutation P (32 → 32)
        int perm = 0;
        for (int i = 0; i < 32; ++i) {
            int src = PBOX[i] - 1;
            int bit = (sOut >> (31 - src)) & 1;
            perm = (perm << 1) | bit;
        }
        return perm;
    }
};

/* -------------------------------------------------------------
   Test driver – small predefined plaintext, keys, IV
   ------------------------------------------------------------- */
int main() {
    // Plaintext: three 8‑byte blocks with varied values
    int plain[24] = {
        // Block 1: "Test1234"
        0x54,0x65,0x73,0x74,0x31,0x32,0x33,0x34,
        // Block 2: "DataABCD"
        0x44,0x61,0x74,0x61,0x41,0x42,0x43,0x44,
        // Block 3: "XYZ!@#$%"
        0x58,0x59,0x5A,0x21,0x40,0x23,0x24,0x25
    };
    // Three 8‑byte keys with mixed patterns
    int keyA[8] = {0x00,0xFF,0xAA,0x55,0x33,0xCC,0x77,0x88};
    int keyB[8] = {0x10,0x20,0x30,0x40,0x50,0x60,0x70,0x80};
    int keyC[8] = {0xFF,0xEE,0xDD,0xCC,0xBB,0xAA,0x99,0x88};
    // IV with a non‑zero pattern
    int iv[8] = {0x12,0x34,0x56,0x78,0x9A,0xBC,0xDE,0xF0};

    int cipher[24] = {0};

    TripleDES_CBC engine;
    engine.setKeySet(keyA, keyB, keyC);
    engine.encrypt(plain, cipher, 3, iv);

    // Output ciphertext in hex
    std::cout << "Ciphertext (hex): ";
    for (int i = 0; i < 24; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (cipher[i] & 0xFF);
    }
    std::cout << std::dec << std::endl;
    return 0;
}
