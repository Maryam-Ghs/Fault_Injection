/* LLM input variant 10: large-safe-stress */
/* 3DES‑EDE implementation – version 10
 *  No dynamic memory, only int and float types.
 *  Deterministic test vectors, manual round unrolling,
 *  fused expressions, stack‑allocated arrays, helper functions.
 */

#include <iostream>
#include <iomanip>
#include <cstdio>
#include <cstdint>

/*--- Permutation tables ---------------------------------------------------*/
int IP[64] = {
 58,50,42,34,26,18,10,2, 60,52,44,36,28,20,12,4,
 62,54,46,38,30,22,14,6, 64,56,48,40,32,24,16,8,
 57,49,41,33,25,17,9,1, 59,51,43,35,27,19,11,3,
 61,53,45,37,29,21,13,5, 63,55,47,39,31,23,15,7};

int FP[64] = {
 40,8,48,16,56,24,64,32, 39,7,47,15,55,23,63,31,
 38,6,46,14,54,22,62,30, 37,5,45,13,53,21,61,29,
 36,4,44,12,52,20,60,28, 35,3,43,11,51,19,59,27,
 34,2,42,10,50,18,58,26, 33,1,41,9,49,17,57,25};

int E[48] = {
 32,1,2,3,4,5, 4,5,6,7,8,9, 8,9,10,11,12,13,
 12,13,14,15,16,17, 16,17,18,19,20,21, 20,21,22,23,24,25,
 24,25,26,27,28,29, 28,29,30,31,32,1};

int P[32] = {
 16,7,20,21, 29,12,28,17,
 1,15,23,26, 5,18,31,10,
 2,8,24,14, 32,27,3,9,
 19,13,30,6, 22,11,4,25};

int PC1[56] = {
 57,49,41,33,25,17,9,
 1,58,50,42,34,26,18,
 10,2,59,51,43,35,27,
 19,11,3,60,52,44,36,
 63,55,47,39,31,23,15,
 7,62,54,46,38,30,22,
 14,6,61,53,45,37,29,
 21,13,5,28,20,12,4};

int PC2[48] = {
 14,17,11,24,1,5,
 3,28,15,6,21,10,
 23,19,12,4,26,8,
 16,7,27,20,13,2,
 41,52,31,37,47,55,
 30,40,51,45,33,48,
 44,49,39,56,34,53,
 46,42,50,36,29,32};

int SHIFTS[16] = {1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1};

/*--- S‑boxes -------------------------------------------------------------*/
int SBOX[8][64] = {
    {14,4,13,1,2,15,11,8,3,10,6,12,5,9,0,7,
     0,15,7,4,14,2,13,1,10,6,12,11,9,5,3,8,
     4,1,14,8,13,6,2,11,15,12,9,7,3,10,5,0,
     15,12,8,2,4,9,1,7,5,11,3,14,10,0,6,13},
    {15,1,8,14,6,11,3,4,9,7,2,13,12,0,5,10,
     3,13,4,7,15,2,8,14,12,0,1,10,6,9,11,5,
     0,14,7,11,10,4,13,1,5,8,12,6,9,3,2,15,
     13,8,10,1,3,15,4,2,11,6,7,12,0,5,14,9},
    {10,0,9,14,6,3,15,5,1,13,12,7,11,4,2,8,
     13,7,0,9,3,4,6,10,2,8,5,14,12,11,15,1,
     13,6,4,9,8,15,3,0,11,1,2,12,5,10,14,7,
     1,10,13,0,6,9,8,7,4,15,14,3,11,5,2,12},
    {7,13,14,3,0,6,9,10,1,2,8,5,11,12,4,15,
     13,8,11,5,6,15,0,3,4,7,2,12,1,10,14,9,
     10,6,9,0,12,11,7,13,15,1,3,14,5,2,8,4,
     3,15,0,6,10,1,13,8,9,4,5,11,12,7,2,14},
    {2,12,4,1,7,10,11,6,8,5,3,15,13,0,14,9,
     14,11,2,12,4,7,13,1,5,0,15,10,3,9,8,6,
     4,2,1,11,10,13,7,8,15,9,12,5,6,3,0,14,
     11,8,12,7,1,14,2,13,6,15,0,9,10,4,5,3},
    {12,1,10,15,9,2,6,8,0,13,3,4,14,7,5,11,
     10,15,4,2,7,12,9,5,6,1,13,14,0,11,3,8,
     9,14,15,5,2,8,12,3,7,0,4,10,1,13,11,6,
     4,3,2,12,9,5,15,10,11,14,1,7,6,0,8,13},
    {4,11,2,14,15,0,8,13,3,12,9,7,5,10,6,1,
     13,0,11,7,4,9,1,10,14,3,5,12,2,15,8,6,
     1,4,11,13,12,3,7,14,10,15,6,8,0,5,9,2,
     6,11,13,8,1,4,10,7,9,5,0,15,14,2,3,12},
    {13,2,8,4,6,15,11,1,10,9,3,14,5,0,12,7,
     1,15,13,8,10,3,7,4,12,5,6,11,0,14,9,2,
     7,11,4,1,9,12,14,2,0,6,10,13,15,3,5,8,
     2,1,14,7,4,10,8,13,15,12,9,0,3,5,6,11}
};

/*--- Helper functions -----------------------------------------------------*/
void bitsFromHex(const char* hex, int* bits, int nbits) {
    for (int i = 0; i < nbits; ++i) bits[i] = 0;
    for (int i = 0; i < nbits/4; ++i) {
        char c = hex[i];
        int v = (c >= '0' && c <= '9') ? c - '0' :
                (c >= 'A' && c <= 'F') ? c - 'A' + 10 :
                c - 'a' + 10;
        for (int b = 0; b < 4; ++b)
            bits[4*i + b] = (v >> (3-b)) & 1;
    }
}
void hexFromBits(const int* bits, int nbits, char* out) {
    for (int i = 0; i < nbits/4; ++i) {
        int v = 0;
        for (int b = 0; b < 4; ++b) v = (v << 1) | bits[4*i + b];
        out[i] = (v < 10) ? ('0' + v) : ('A' + v - 10);
    }
    out[nbits/4] = '\0';
}
void permute(const int* in, const int* table, int* out, int n) {
    for (int i = 0; i < n; ++i) out[i] = in[table[i]-1];
}
void leftRotate28(int* half, int steps) {
    int tmp[28];
    for (int i = 0; i < 28; ++i) tmp[i] = half[(i+steps)%28];
    for (int i = 0; i < 28; ++i) half[i] = tmp[i];
}
void generateSubkeys(const int* keybits, int subkeys[16][48]) {
    int pc1[56]; permute(keybits, PC1, pc1, 56);
    int C[28], D[28];
    for (int i = 0; i < 28; ++i) C[i] = pc1[i];
    for (int i = 0; i < 28; ++i) D[i] = pc1[28+i];
    for (int r = 0; r < 16; ++r) {
        leftRotate28(C, SHIFTS[r]);
        leftRotate28(D, SHIFTS[r]);
        int CD[56];
        for (int i = 0; i < 28; ++i) CD[i] = C[i];
        for (int i = 0; i < 28; ++i) CD[28+i] = D[i];
        permute(CD, PC2, subkeys[r], 48);
    }
}
void feistel(const int* R, const int* subkey, int* out) {
    int expanded[48]; permute(R, E, expanded, 48);
    for (int i = 0; i < 48; ++i) expanded[i] ^= subkey[i];
    int soutput[32];
    for (int s = 0; s < 8; ++s) {
        int row = (expanded[6*s] << 1) | expanded[6*s+5];
        int col = (expanded[6*s+1] << 3) |
                  (expanded[6*s+2] << 2) |
                  (expanded[6*s+3] << 1) |
                  expanded[6*s+4];
        int val = SBOX[s][row*16 + col];
        for (int b = 0; b < 4; ++b)
            soutput[4*s + b] = (val >> (3-b)) & 1;
    }
    permute(soutput, P, out, 32);
}
void encryptBlock(const int* in, const int subkeys[16][48], int* out) {
    int ip[64]; permute(in, IP, ip, 64);
    int L[32], R[32];
    for (int i = 0; i < 32; ++i) L[i] = ip[i];
    for (int i = 0; i < 32; ++i) R[i] = ip[32+i];

    /* ---- 16 rounds – manual unrolling --------------------------------- */
    int F[32];

    feistel(R, subkeys[0], F);
    for (int i = 0; i < 32; ++i) { int tmp = L[i]; L[i] = R[i]; R[i] = tmp ^ F[i]; }

    feistel(R, subkeys[1], F);
    for (int i = 0; i < 32; ++i) { int tmp = L[i]; L[i] = R[i]; R[i] = tmp ^ F[i]; }

    feistel(R, subkeys[2], F);
    for (int i = 0; i < 32; ++i) { int tmp = L[i]; L[i] = R[i]; R[i] = tmp ^ F[i]; }

    feistel(R, subkeys[3], F);
    for (int i = 0; i < 32; ++i) { int tmp = L[i]; L[i] = R[i]; R[i] = tmp ^ F[i]; }

    feistel(R, subkeys[4], F);
    for (int i = 0; i < 32; ++i) { int tmp = L[i]; L[i] = R[i]; R[i] = tmp ^ F[i]; }

    feistel(R, subkeys[5], F);
    for (int i = 0; i < 32; ++i) { int tmp = L[i]; L[i] = R[i]; R[i] = tmp ^ F[i]; }

    feistel(R, subkeys[6], F);
    for (int i = 0; i < 32; ++i) { int tmp = L[i]; L[i] = R[i]; R[i] = tmp ^ F[i]; }

    feistel(R, subkeys[7], F);
    for (int i = 0; i < 32; ++i) { int tmp = L[i]; L[i] = R[i]; R[i] = tmp ^ F[i]; }

    feistel(R, subkeys[8], F);
    for (int i = 0; i < 32; ++i) { int tmp = L[i]; L[i] = R[i]; R[i] = tmp ^ F[i]; }

    feistel(R, subkeys[9], F);
    for (int i = 0; i < 32; ++i) { int tmp = L[i]; L[i] = R[i]; R[i] = tmp ^ F[i]; }

    feistel(R, subkeys[10], F);
    for (int i = 0; i < 32; ++i) { int tmp = L[i]; L[i] = R[i]; R[i] = tmp ^ F[i]; }

    feistel(R, subkeys[11], F);
    for (int i = 0; i < 32; ++i) { int tmp = L[i]; L[i] = R[i]; R[i] = tmp ^ F[i]; }

    feistel(R, subkeys[12], F);
    for (int i = 0; i < 32; ++i) { int tmp = L[i]; L[i] = R[i]; R[i] = tmp ^ F[i]; }

    feistel(R, subkeys[13], F);
    for (int i = 0; i < 32; ++i) { int tmp = L[i]; L[i] = R[i]; R[i] = tmp ^ F[i]; }

    feistel(R, subkeys[14], F);
    for (int i = 0; i < 32; ++i) { int tmp = L[i]; L[i] = R[i]; R[i] = tmp ^ F[i]; }

    feistel(R, subkeys[15], F);
    for (int i = 0; i < 32; ++i) { int tmp = L[i]; L[i] = R[i]; R[i] = tmp ^ F[i]; }
    /* ------------------------------------------------------------------- */

    int preoutput[64];
    for (int i = 0; i < 32; ++i) preoutput[i] = R[i];
    for (int i = 0; i < 32; ++i) preoutput[32+i] = L[i];
    permute(preoutput, FP, out, 64);
}
void decryptBlock(const int* in, const int subkeys[16][48], int* out) {
    int ip[64]; permute(in, IP, ip, 64);
    int L[32], R[32];
    for (int i = 0; i < 32; ++i) L[i] = ip[i];
    for (int i = 0; i < 32; ++i) R[i] = ip[32+i];

    int F[32];
    /* ---- 16 rounds – manual unrolling (reverse subkeys) -------------- */
    for (int r = 15; r >= 0; --r) {
        feistel(R, subkeys[r], F);
        for (int i = 0; i < 32; ++i) { int tmp = L[i]; L[i] = R[i]; R[i] = tmp ^ F[i]; }
    }
    /* ------------------------------------------------------------------- */

    int preoutput[64];
    for (int i = 0; i < 32; ++i) preoutput[i] = R[i];
    for (int i = 0; i < 32; ++i) preoutput[32+i] = L[i];
    permute(preoutput, FP, out, 64);
}
void tripleDES(const int* plain, const int subK1[16][48],
               const int subK2[16][48], const int subK3[16][48],
               int* cipher) {
    int step1[64], step2[64];
    encryptBlock(plain, subK1, step1);
    decryptBlock(step1, subK2, step2);
    encryptBlock(step2, subK3, cipher);
}

/*--- Main -----------------------------------------------------------------*/
int main() {
    /* Deterministic test vectors (hex strings) – larger stress set */
    const char* keyHex1 = "0123456789ABCDEF";
    const char* keyHex2 = "23456789ABCDEF01";
    const char* keyHex3 = "456789ABCDEF0123";

    const int BLOCK_COUNT = 256;               // reasonable stress size
    int ptBits[BLOCK_COUNT][64];
    int ctBits[BLOCK_COUNT][64];
    char ptHex[17];
    char ctHex[17];

    int keyBits1[64], keyBits2[64], keyBits3[64];
    bitsFromHex(keyHex1, keyBits1, 64);
    bitsFromHex(keyHex2, keyBits2, 64);
    bitsFromHex(keyHex3, keyBits3, 64);

    int subK1[16][48], subK2[16][48], subK3[16][48];
    generateSubkeys(keyBits1, subK1);
    generateSubkeys(keyBits2, subK2);
    generateSubkeys(keyBits3, subK3);

    /* Generate a sequence of plaintext blocks: 0x0000000000000000, 0x0000000000000001, ... */
    for (int i = 0; i < BLOCK_COUNT; ++i) {
        std::uint64_t val = static_cast<std::uint64_t>(i);
        std::snprintf(ptHex, sizeof(ptHex), "%016llX", static_cast<unsigned long long>(val));
        bitsFromHex(ptHex, ptBits[i], 64);
    }

    /* Encrypt all blocks */
    for (int i = 0; i < BLOCK_COUNT; ++i) {
        tripleDES(ptBits[i], subK1, subK2, subK3, ctBits[i]);
    }

    /* Output summary – first, last, and total count */
    std::cout << "Processed " << BLOCK_COUNT << " blocks with 3DES‑EDE.\n";
    std::cout << "Key 1     : " << keyHex1 << std::endl;
    std::cout << "Key 2     : " << keyHex2 << std::endl;
    std::cout << "Key 3     : " << keyHex3 << std::endl;

    hexFromBits(ctBits[0], 64, ctHex);
    std::cout << "Ciphertext of first block  : " << ctHex << std::endl;

    hexFromBits(ctBits[BLOCK_COUNT-1], 64, ctHex);
    std::cout << "Ciphertext of last block   : " << ctHex << std::endl;

    return 0;
}
