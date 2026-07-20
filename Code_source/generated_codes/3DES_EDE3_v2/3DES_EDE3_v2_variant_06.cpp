#include <iostream>
using namespace std;

/* LLM input variant 6: ordered-structured */

int main() {
    /* ---------- tables (no const) ---------- */
    int IP[64] = {
        58,50,42,34,26,18,10,2, 60,52,44,36,28,20,12,4,
        62,54,46,38,30,22,14,6, 64,56,48,40,32,24,16,8,
        57,49,41,33,25,17,9,1, 59,51,43,35,27,19,11,3,
        61,53,45,37,29,21,13,5, 63,55,47,39,31,23,15,7
    };
    int FP[64] = {
        40,8,48,16,56,24,64,32, 39,7,47,15,55,23,63,31,
        38,6,46,14,54,22,62,30, 37,5,45,13,53,21,61,29,
        36,4,44,12,52,20,60,28, 35,3,43,11,51,19,59,27,
        34,2,42,10,50,18,58,26, 33,1,41,9,49,17,57,25
    };
    int E[48] = {
        32,1,2,3,4,5, 4,5,6,7,8,9,
        8,9,10,11,12,13, 12,13,14,15,16,17,
        16,17,18,19,20,21, 20,21,22,23,24,25,
        24,25,26,27,28,29, 28,29,30,31,32,1
    };
    int P[32] = {
        16,7,20,21, 29,12,28,17,
        1,15,23,26, 5,18,31,10,
        2,8,24,14, 32,27,3,9,
        19,13,30,6, 22,11,4,25
    };
    int PC1[56] = {
        57,49,41,33,25,17,9, 1,58,50,42,34,26,18,
        10,2,59,51,43,35,27,19, 11,3,60,52,44,36,
        63,55,47,39,31,23,15, 7,62,54,46,38,30,22,
        14,6,61,53,45,37,29,21, 13,5,28,20,12,4
    };
    int PC2[48] = {
        14,17,11,24,1,5, 3,28,15,6,21,10,
        23,19,12,4,26,8, 16,7,27,20,13,2,
        41,52,31,37,47,55, 30,40,51,45,33,48,
        44,49,39,56,34,53, 46,42,50,36,29,32
    };
    int SH[16] = {1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1};

    int S[8][4][16] = {
        {{14,4,13,1,2,15,11,8,3,10,6,12,5,9,0,7},
         {0,15,7,4,14,2,13,1,10,6,12,11,9,5,3,8},
         {4,1,14,8,13,6,2,11,15,12,9,7,3,10,5,0},
         {15,12,8,2,4,9,1,7,5,11,3,14,10,0,6,13}},
        {{15,1,8,14,6,11,3,4,9,7,2,13,12,0,5,10},
         {3,13,4,7,15,2,8,14,12,0,1,10,6,9,11,5},
         {0,14,7,11,10,4,13,1,5,8,12,6,9,3,2,15},
         {13,8,10,1,3,15,4,2,11,6,7,12,0,5,14,9}},
        {{10,0,9,14,6,3,15,5,1,13,12,7,11,4,2,8},
         {13,7,0,9,3,4,6,10,2,8,5,14,12,11,15,1},
         {13,6,4,9,8,15,3,0,11,1,2,12,5,10,14,7},
         {1,10,13,0,6,9,8,7,4,15,14,3,11,5,2,12}},
        {{7,13,14,3,0,6,9,10,1,2,8,5,11,12,4,15},
         {13,8,11,5,6,15,0,3,4,7,2,12,1,10,14,9},
         {10,6,9,0,12,11,7,13,15,1,3,14,5,2,8,4},
         {3,15,0,6,10,1,13,8,9,4,5,11,12,7,2,14}},
        {{2,12,4,1,7,10,11,6,8,5,3,15,13,0,14,9},
         {14,11,2,12,4,7,13,1,5,0,15,10,3,9,8,6},
         {4,2,1,11,10,13,7,8,15,9,12,5,6,3,0,14},
         {11,8,12,7,1,14,2,13,6,15,0,9,10,4,5,3}},
        {{12,1,10,15,9,2,6,8,0,13,3,4,14,7,5,11},
         {10,15,4,2,7,12,9,5,6,1,13,14,0,11,3,8},
         {9,14,15,5,2,8,12,3,7,0,4,10,1,13,11,6},
         {4,3,2,12,9,5,15,10,11,14,1,7,6,0,8,13}},
        {{4,11,2,14,15,0,8,13,3,12,9,7,5,10,6,1},
         {13,0,11,7,4,9,1,10,14,3,5,12,2,15,8,6},
         {1,4,11,13,12,3,7,14,10,15,6,8,0,5,9,2},
         {6,11,13,8,1,4,10,7,9,5,0,15,14,2,3,12}},
        {{13,2,8,4,6,15,11,1,10,9,3,14,5,0,12,7},
         {1,15,13,8,10,3,7,4,12,5,6,11,0,14,9,2},
         {7,11,4,1,9,12,14,2,0,6,10,13,15,3,5,8},
         {2,1,14,7,4,10,8,13,15,12,9,0,3,5,6,11}}
    };

    /* ---------- helper lambdas ---------- */
    auto permute = [&](int *src, int *dst, int *tbl, int n) {
        int i = 0;
        while (i < n) {
            dst[i] = src[tbl[i] - 1];
            ++i;
        }
    };

    auto leftShift = [&](int *half, int n) {
        int tmp[28];
        int i = 0;
        while (i < 28) {
            tmp[i] = half[(i + n) % 28];
            ++i;
        }
        i = 0;
        while (i < 28) {
            half[i] = tmp[i];
            ++i;
        }
    };

    auto fFunc = [&](int *R, int *subk, int *out) {
        int eR[48];
        permute(R, eR, E, 48);
        int i = 0;
        while (i < 48) {
            eR[i] ^= subk[i];
            ++i;
        }
        int sOut[32];
        i = 0;
        while (i < 8) {
            int six = 0;
            int j = 0;
            while (j < 6) {
                six = (six << 1) | eR[i * 6 + j];
                ++j;
            }
            int row = ((six & 0x20) >> 4) | (six & 0x01);
            int col = (six >> 1) & 0x0F;
            int val = S[i][row][col];
            int k = 0;
            while (k < 4) {
                sOut[i * 4 + (3 - k)] = (val >> k) & 1;
                ++k;
            }
            ++i;
        }
        permute(sOut, out, P, 32);
    };

    auto generateSubKeys = [&](int *keyBytes, int subKeys[16][48]) {
        int keyBits[64];
        int i = 0;
        while (i < 8) {
            int j = 0;
            while (j < 8) {
                keyBits[i * 8 + (7 - j)] = (keyBytes[i] >> j) & 1;
                ++j;
            }
            ++i;
        }
        int pc1bits[56];
        permute(keyBits, pc1bits, PC1, 56);
        int C[28], D[28];
        i = 0;
        while (i < 28) { C[i] = pc1bits[i]; D[i] = pc1bits[i + 28]; ++i; }
        int round = 0;
        while (round < 16) {
            leftShift(C, SH[round]);
            leftShift(D, SH[round]);
            int CD[56];
            i = 0;
            while (i < 28) { CD[i] = C[i]; CD[i + 28] = D[i]; ++i; }
            permute(CD, subKeys[round], PC2, 48);
            ++round;
        }
    };

    auto desBlock = [&](int *blockBytes, int subKeys[16][48], bool encrypt, int *outBytes) {
        int blockBits[64];
        int i = 0;
        while (i < 8) {
            int j = 0;
            while (j < 8) {
                blockBits[i * 8 + (7 - j)] = (blockBytes[i] >> j) & 1;
                ++j;
            }
            ++i;
        }
        int ipBits[64];
        permute(blockBits, ipBits, IP, 64);
        int L[32], R[32];
        i = 0;
        while (i < 32) { L[i] = ipBits[i]; R[i] = ipBits[i + 32]; ++i; }
        int round = 0;
        while (round < 16) {
            int fOut[32];
            fFunc(R, subKeys[encrypt ? round : 15 - round], fOut);
            int newR[32];
            i = 0;
            while (i < 32) {
                newR[i] = L[i] ^ fOut[i];
                ++i;
            }
            i = 0;
            while (i < 32) { L[i] = R[i]; R[i] = newR[i]; ++i; }
            ++round;
        }
        int preFP[64];
        i = 0;
        while (i < 32) { preFP[i] = R[i]; preFP[i + 32] = L[i]; ++i; }
        int fpBits[64];
        permute(preFP, fpBits, FP, 64);
        i = 0;
        while (i < 8) {
            int val = 0;
            int j = 0;
            while (j < 8) {
                val = (val << 1) | fpBits[i * 8 + j];
                ++j;
            }
            outBytes[i] = val;
            ++i;
        }
    };

    /* ---------- deterministic test vectors ---------- */
    int K1[8] = {0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77};
    int K2[8] = {0x88,0x99,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF};
    int K3[8] = {0xFF,0xEE,0xDD,0xCC,0xBB,0xAA,0x99,0x88};
    int PT[8] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07};

    /* ---------- key schedules ---------- */
    int subK1[16][48], subK2[16][48], subK3[16][48];
    generateSubKeys(K1, subK1);
    generateSubKeys(K2, subK2);
    generateSubKeys(K3, subK3);

    /* ---------- 3DES EDE encryption ---------- */
    int tmp1[8], tmp2[8], CT[8];
    desBlock(PT, subK1, true,  tmp1);   // Encrypt with K1
    desBlock(tmp1, subK2, false, tmp2); // Decrypt with K2
    desBlock(tmp2, subK3, true,  CT);   // Encrypt with K3

    /* ---------- output ---------- */
    cout << "Plaintext : ";
    int i = 0;
    while (i < 8) {
        printf("%02X", PT[i]);
        ++i;
    }
    cout << "\nCiphertext: ";
    i = 0;
    while (i < 8) {
        printf("%02X", CT[i]);
        ++i;
    }
    cout << endl;
    return 0;
}
