#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>

/* LLM input variant 10: large-safe-stress */

int main()
{
    /* -------------------------------------------------------------
       Helper tables – all stored as plain int arrays (no const).
       ------------------------------------------------------------- */
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

    int S[8][4][16] = {
        { // S1
            {14,4,13,1,2,15,11,8,3,10,6,12,5,9,0,7},
            {0,15,7,4,14,2,13,1,10,6,12,11,9,5,3,8},
            {4,1,14,8,13,6,2,11,15,12,9,7,3,10,5,0},
            {15,12,8,2,4,9,1,7,5,11,3,14,10,0,6,13}
        },
        { // S2
            {15,1,8,14,6,11,3,4,9,7,2,13,12,0,5,10},
            {3,13,4,7,15,2,8,14,12,0,1,10,6,9,11,5},
            {0,14,7,11,10,4,13,1,5,8,12,6,9,3,2,15},
            {13,8,10,1,3,15,4,2,11,6,7,12,0,5,14,9}
        },
        { // S3
            {10,0,9,14,6,3,15,5,1,13,12,7,11,4,2,8},
            {13,7,0,9,3,4,6,10,2,8,5,14,12,11,15,1},
            {13,6,4,9,8,15,3,0,11,1,2,12,5,10,14,7},
            {1,10,13,0,6,9,8,7,4,15,14,3,11,5,2,12}
        },
        { // S4
            {7,13,14,3,0,6,9,10,1,2,8,5,11,12,4,15},
            {13,8,11,5,6,15,0,3,4,7,2,12,1,10,14,9},
            {10,6,9,0,12,11,7,13,15,1,3,14,5,2,8,4},
            {3,15,0,6,10,1,13,8,9,4,5,11,12,7,2,14}
        },
        { // S5
            {2,12,4,1,7,10,11,6,8,5,3,15,13,0,14,9},
            {14,11,2,12,4,7,13,1,5,0,15,10,3,9,8,6},
            {4,2,1,11,10,13,7,8,15,9,12,5,6,3,0,14},
            {11,8,12,7,1,14,2,13,6,15,0,9,10,4,5,3}
        },
        { // S6
            {12,1,10,15,9,2,6,8,0,13,3,4,14,7,5,11},
            {10,15,4,2,7,12,9,5,6,1,13,14,0,11,3,8},
            {9,14,15,5,2,8,12,3,7,0,4,10,1,13,11,6},
            {4,3,2,12,9,5,15,10,11,14,1,7,6,0,8,13}
        },
        { // S7
            {4,11,2,14,15,0,8,13,3,12,9,7,5,10,6,1},
            {13,0,11,7,4,9,1,10,14,3,5,12,2,15,8,6},
            {1,4,11,13,12,3,7,14,10,15,6,8,0,5,9,2},
            {6,11,13,8,1,4,10,7,9,5,0,15,14,2,3,12}
        },
        { // S8
            {13,2,8,4,6,15,11,1,10,9,3,14,5,0,12,7},
            {1,15,13,8,10,3,7,4,12,5,6,11,0,14,9,2},
            {7,11,4,1,9,12,14,2,0,6,10,13,15,3,5,8},
            {2,1,14,7,4,10,8,13,15,12,9,0,3,5,6,11}
        }
    };

    /* -------------------------------------------------------------
       Helper lambdas – defined inside main so that everything stays
       within the main function as required.
       ------------------------------------------------------------- */
    auto permute = [&](int *in, int *out, int *table, int n) {
        for (int i = 0; i < n; ++i) {
            out[i] = in[ table[i] - 1 ];
        }
    };

    auto leftRotate = [&](int *bits, int len, int cnt) {
        int tmp[28];
        for (int i = 0; i < len; ++i) tmp[i] = bits[i];
        for (int i = 0; i < len; ++i) {
            bits[i] = tmp[(i + cnt) % len];
        }
    };

    auto xorBits = [&](int *a, int *b, int *out, int n) {
        for (int i = 0; i < n; ++i) out[i] = a[i] ^ b[i];
    };

    auto fFunction = [&](int *R, int *subkey, int *out) {
        int expanded[48];
        permute(R, expanded, E, 48);
        xorBits(expanded, subkey, expanded, 48);

        int sOut[32];
        for (int box = 0; box < 8; ++box) {
            int row = (expanded[box*6] << 1) | expanded[box*6 + 5];
            int col = (expanded[box*6 + 1] << 3) |
                      (expanded[box*6 + 2] << 2) |
                      (expanded[box*6 + 3] << 1) |
                      expanded[box*6 + 4];
            int val = S[box][row][col];
            sOut[box*4 + 0] = (val >> 3) & 1;
            sOut[box*4 + 1] = (val >> 2) & 1;
            sOut[box*4 + 2] = (val >> 1) & 1;
            sOut[box*4 + 3] =  val       & 1;
        }
        permute(sOut, out, P, 32);
    };

    auto generateSubkeys = [&](int *key64, int subkeys[16][48]) {
        int key56[56];
        permute(key64, key56, PC1, 56);
        int C[28], D[28];
        for (int i = 0; i < 28; ++i) {
            C[i] = key56[i];
            D[i] = key56[28 + i];
        }
        for (int round = 0; round < 16; ++round) {
            leftRotate(C, 28, SHIFTS[round]);
            leftRotate(D, 28, SHIFTS[round]);
            int CD[56];
            for (int i = 0; i < 28; ++i) {
                CD[i] = C[i];
                CD[28 + i] = D[i];
            }
            permute(CD, subkeys[round], PC2, 48);
        }
    };

    auto desEncrypt = [&](int *plain64, int subkeys[16][48], int *cipher64) {
        int ip[64];
        permute(plain64, ip, IP, 64);
        int L[32], R[32];
        for (int i = 0; i < 32; ++i) {
            L[i] = ip[i];
            R[i] = ip[32 + i];
        }
        for (int round = 0; round < 16; ++round) {
            int fOut[32];
            fFunction(R, subkeys[round], fOut);
            int newR[32];
            xorBits(L, fOut, newR, 32);
            for (int i = 0; i < 32; ++i) {
                L[i] = R[i];
                R[i] = newR[i];
            }
        }
        int preFP[64];
        for (int i = 0; i < 32; ++i) {
            preFP[i] = R[i];
            preFP[32 + i] = L[i];
        }
        permute(preFP, cipher64, FP, 64);
    };

    auto desDecrypt = [&](int *cipher64, int subkeys[16][48], int *plain64) {
        int ip[64];
        permute(cipher64, ip, IP, 64);
        int L[32], R[32];
        for (int i = 0; i < 32; ++i) {
            L[i] = ip[i];
            R[i] = ip[32 + i];
        }
        for (int round = 15; round >= 0; --round) {
            int fOut[32];
            fFunction(R, subkeys[round], fOut);
            int newR[32];
            xorBits(L, fOut, newR, 32);
            for (int i = 0; i < 32; ++i) {
                L[i] = R[i];
                R[i] = newR[i];
            }
        }
        int preFP[64];
        for (int i = 0; i < 32; ++i) {
            preFP[i] = R[i];
            preFP[32 + i] = L[i];
        }
        permute(preFP, plain64, FP, 64);
    };

    auto bitsToHexString = [&](int *bits) {
        char hexDigits[] = "0123456789ABCDEF";
        std::string out;
        for (int i = 0; i < 64; i += 4) {
            int val = (bits[i] << 3) |
                      (bits[i+1] << 2) |
                      (bits[i+2] << 1) |
                      bits[i+3];
            out.push_back(hexDigits[val]);
        }
        return out;
    };

    /* -------------------------------------------------------------
       Stress test: generate many 64‑bit plaintext blocks.
       ------------------------------------------------------------- */
    const int BLOCKS = 1024; // large but safe
    std::srand( static_cast<unsigned>( std::time(0) ) );

    int plain[BLOCKS][64];
    int key1[64], key2[64], key3[64];
    for (int i = 0; i < 64; ++i) {
        key1[i] = std::rand() % 2;
        key2[i] = std::rand() % 2;
        key3[i] = std::rand() % 2;
    }
    for (int b = 0; b < BLOCKS; ++b) {
        for (int i = 0; i < 64; ++i) {
            plain[b][i] = std::rand() % 2;
        }
    }

    /* -------------------------------------------------------------
       Produce subkeys for the three DES keys.
       ------------------------------------------------------------- */
    int subK1[16][48];
    int subK2[16][48];
    int subK3[16][48];
    generateSubkeys(key1, subK1);
    generateSubkeys(key2, subK2);
    generateSubkeys(key3, subK3);

    /* -------------------------------------------------------------
       3DES‑EDE on each block.
       ------------------------------------------------------------- */
    int cipher[BLOCKS][64];
    for (int b = 0; b < BLOCKS; ++b) {
        int step1[64];
        int step2[64];
        desEncrypt(plain[b], subK1, step1);
        desDecrypt(step1, subK2, step2);
        desEncrypt(step2, subK3, cipher[b]);
    }

    /* -------------------------------------------------------------
       Output concatenated hex strings for all blocks.
       ------------------------------------------------------------- */
    std::string allPlainHex, allCipherHex;
    for (int b = 0; b < BLOCKS; ++b) {
        allPlainHex += bitsToHexString(plain[b]);
        allCipherHex += bitsToHexString(cipher[b]);
    }
    std::cout << "Plaintext : " << allPlainHex << std::endl;
    std::cout << "Ciphertext: " << allCipherHex << std::endl;

    return 0;
}
