#include <iostream>

/* LLM input variant 2: small-diverse */
int main()
{
    /* ---------- tables (non‑const, allocated on the heap) ---------- */
    int* IP = new int[64]{
        58,50,42,34,26,18,10,2, 60,52,44,36,28,20,12,4,
        62,54,46,38,30,22,14,6, 64,56,48,40,32,24,16,8,
        57,49,41,33,25,17,9,1, 59,51,43,35,27,19,11,3,
        61,53,45,37,29,21,13,5, 63,55,47,39,31,23,15,7
    };
    int* IPinv = new int[64]{
        40,8,48,16,56,24,64,32, 39,7,47,15,55,23,63,31,
        38,6,46,14,54,22,62,30, 37,5,45,13,53,21,61,29,
        36,4,44,12,52,20,60,28, 35,3,43,11,51,19,59,27,
        34,2,42,10,50,18,58,26, 33,1,41,9,49,17,57,25
    };
    int* PC1 = new int[56]{
        57,49,41,33,25,17,9, 1,58,50,42,34,26,18,
        10,2,59,51,43,35,27,19,11,3,60,52,44,36,
        63,55,47,39,31,23,15,7,62,54,46,38,30,22,
        14,6,61,53,45,37,29,21,13,5,28,20,12,4
    };
    int* PC2 = new int[48]{
        14,17,11,24,1,5, 3,28,15,6,21,10,
        23,19,12,4,26,8, 16,7,27,20,13,2,
        41,52,31,37,47,55, 30,40,51,45,33,48,
        44,49,39,56,34,53, 46,42,50,36,29,32
    };
    int* ShiftSchedule = new int[16]{1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1};
    int* E = new int[48]{
        32,1,2,3,4,5, 4,5,6,7,8,9,
        8,9,10,11,12,13, 12,13,14,15,16,17,
        16,17,18,19,20,21, 20,21,22,23,24,25,
        24,25,26,27,28,29, 28,29,30,31,32,1
    };
    int* P = new int[32]{
        16,7,20,21, 29,12,28,17,
        1,15,23,26, 5,18,31,10,
        2,8,24,14, 32,27,3,9,
        19,13,30,6, 22,11,4,25
    };
    /* S‑boxes (8 boxes, each 4×16) */
    int* S[8];
    for (int i = 0; i < 8; ++i) S[i] = new int[64];
    /* S‑box 1 */
    int s1[64] = {
        14,4,13,1,2,15,11,8,3,10,6,12,5,9,0,7,
        0,15,7,4,14,2,13,1,10,6,12,11,9,5,3,8,
        4,1,14,8,13,6,2,11,15,12,9,7,3,10,5,0,
        15,12,8,2,4,9,1,7,5,11,3,14,10,0,6,13
    };
    for (int i = 0; i < 64; ++i) S[0][i] = s1[i];
    /* S‑box 2 */
    int s2[64] = {
        15,1,8,14,6,11,3,4,9,7,2,13,12,0,5,10,
        3,13,4,7,15,2,8,14,12,0,1,10,6,9,11,5,
        0,14,7,11,10,4,13,1,5,8,12,6,9,3,2,15,
        13,8,10,1,3,15,4,2,11,6,7,12,0,5,14,9
    };
    for (int i = 0; i < 64; ++i) S[1][i] = s2[i];
    /* S‑box 3 */
    int s3[64] = {
        10,0,9,14,6,3,15,5,1,13,12,7,11,4,2,8,
        13,7,0,9,3,4,6,10,2,8,5,14,12,11,15,1,
        13,6,4,9,8,15,3,0,11,1,2,12,5,10,14,7,
        1,10,13,0,6,9,8,7,4,15,14,3,11,5,2,12
    };
    for (int i = 0; i < 64; ++i) S[2][i] = s3[i];
    /* S‑box 4 */
    int s4[64] = {
        7,13,14,3,0,6,9,10,1,2,8,5,11,12,4,15,
        13,8,11,5,6,15,0,3,4,7,2,12,1,10,14,9,
        10,6,9,0,12,11,7,13,15,1,3,14,5,2,8,4,
        3,15,0,6,10,1,13,8,9,4,5,11,12,7,2,14
    };
    for (int i = 0; i < 64; ++i) S[3][i] = s4[i];
    /* S‑box 5 */
    int s5[64] = {
        2,12,4,1,7,10,11,6,8,5,3,15,13,0,14,9,
        14,11,2,12,4,7,13,1,5,0,15,10,3,9,8,6,
        4,2,1,11,10,13,7,8,15,9,12,5,6,3,0,14,
        11,8,12,7,1,14,2,13,6,15,0,9,10,4,5,3
    };
    for (int i = 0; i < 64; ++i) S[4][i] = s5[i];
    /* S‑box 6 */
    int s6[64] = {
        12,1,10,15,9,2,6,8,0,13,3,4,14,7,5,11,
        10,15,4,2,7,12,9,5,6,1,13,14,0,11,3,8,
        9,14,15,5,2,8,12,3,7,0,4,10,1,13,11,6,
        4,3,2,12,9,5,15,10,11,14,1,7,6,0,8,13
    };
    for (int i = 0; i < 64; ++i) S[5][i] = s6[i];
    /* S‑box 7 */
    int s7[64] = {
        4,11,2,14,15,0,8,13,3,12,9,7,5,10,6,1,
        13,0,11,7,4,9,1,10,14,3,5,12,2,15,8,6,
        1,4,11,13,12,3,7,14,10,15,6,8,0,5,9,2,
        6,11,13,8,1,4,10,7,9,5,0,15,14,2,3,12
    };
    for (int i = 0; i < 64; ++i) S[6][i] = s7[i];
    /* S‑box 8 */
    int s8[64] = {
        13,2,8,4,6,15,11,1,10,9,3,14,5,0,12,7,
        1,15,13,8,10,3,7,4,12,5,6,11,0,14,9,2,
        7,11,4,1,9,12,14,2,0,6,10,13,15,3,5,8,
        2,1,14,7,4,10,8,13,15,12,9,0,3,5,6,11
    };
    for (int i = 0; i < 64; ++i) S[7][i] = s8[i];

    /* ---------- test vectors (deterministic) ---------- */
    int key1[8] = {0xFF,0xEE,0xDD,0xCC,0xBB,0xAA,0x99,0x88};
    int key2[8] = {0x88,0x99,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF};
    int key3[8] = {0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77};
    int iv[8]   = {0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF};
    int plain[8]= { 'T','e','s','t','1','2','3','!' };

    /* ---------- helper lambdas (inline, no extra functions) ---------- */
    auto bytesToBits = [&](int* srcBytes, int* dstBits){
        for (int i = 0; i < 8; ++i){
            for (int b = 0; b < 8; ++b){
                dstBits[i*8 + b] = (srcBytes[i] >> (7-b)) & 1;
            }
        }
    };
    auto bitsToBytes = [&](int* srcBits, int* dstBytes){
        for (int i = 0; i < 8; ++i){
            int val = 0;
            for (int b = 0; b < 8; ++b){
                val = (val << 1) | srcBits[i*8 + b];
            }
            dstBytes[i] = val;
        }
    };
    auto permute = [&](int* src, int* table, int n, int* dst){
        for (int i = 0; i < n; ++i){
            dst[i] = src[table[i]-1];
        }
    };
    auto leftShift = [&](int* arr, int len, int cnt){
        int* tmp = new int[len];
        for (int i = 0; i < len; ++i){
            tmp[i] = arr[(i+cnt)%len];
        }
        for (int i = 0; i < len; ++i) arr[i] = tmp[i];
        delete[] tmp;
    };

    /* ---------- key schedule for a single DES key ---------- */
    auto generateSubkeys = [&](int* keyBytes, int* subkeys){
        int keyBits[64];
        bytesToBits(keyBytes, keyBits);
        int permuted[56];
        permute(keyBits, PC1, 56, permuted);
        int C[28], D[28];
        for (int i = 0; i < 28; ++i){ C[i] = permuted[i]; D[i] = permuted[28+i]; }
        for (int round = 0; round < 16; ++round){
            leftShift(C,28,ShiftSchedule[round]);
            leftShift(D,28,ShiftSchedule[round]);
            int CD[56];
            for (int i = 0; i < 28; ++i){ CD[i]=C[i]; CD[28+i]=D[i]; }
            permute(CD, PC2, 48, &subkeys[round*48]);
        }
    };

    /* ---------- generate subkeys for three keys ---------- */
    int* subK1 = new int[16*48];
    int* subK2 = new int[16*48];
    int* subK3 = new int[16*48];
    generateSubkeys(key1, subK1);
    generateSubkeys(key2, subK2);
    generateSubkeys(key3, subK3);

    /* ---------- DES round function (F) ---------- */
    auto feistel = [&](int* R, int* subkey, int* out){
        int expanded[48];
        permute(R, E, 48, expanded);
        for (int i = 0; i < 48; ++i) expanded[i] ^= subkey[i];
        int sOut[32];
        for (int box = 0; box < 8; ++box){
            int six[6];
            for (int i = 0; i < 6; ++i) six[i] = expanded[box*6+i];
            int row = (six[0]<<1) | six[5];
            int col = (six[1]<<3) | (six[2]<<2) | (six[3]<<1) | six[4];
            int sVal = S[box][row*16 + col];
            for (int b = 0; b < 4; ++b){
                sOut[box*4 + b] = (sVal >> (3-b)) & 1;
            }
        }
        permute(sOut, P, 32, out);
    };

    /* ---------- DES encrypt (single key) ---------- */
    auto desEncrypt = [&](int* inBits, int* subkeys, int* outBits){
        int permuted[64];
        permute(inBits, IP, 64, permuted);
        int L[32], R[32];
        for (int i = 0; i < 32; ++i){ L[i]=permuted[i]; R[i]=permuted[32+i]; }
        for (int round = 0; round < 16; ++round){
            int newR[32];
            feistel(R, &subkeys[round*48], newR);
            int newL[32];
            for (int i = 0; i < 32; ++i) newL[i] = R[i];
            for (int i = 0; i < 32; ++i) R[i] = L[i] ^ newR[i];
            for (int i = 0; i < 32; ++i) L[i] = newL[i];
        }
        int preout[64];
        for (int i = 0; i < 32; ++i){ preout[i]=R[i]; }
        for (int i = 0; i < 32; ++i){ preout[32+i]=L[i]; }
        permute(preout, IPinv, 64, outBits);
    };

    /* ---------- DES decrypt (single key) ---------- */
    auto desDecrypt = [&](int* inBits, int* subkeys, int* outBits){
        int revSub[16*48];
        for (int r = 0; r < 16; ++r){
            for (int i = 0; i < 48; ++i){
                revSub[r*48 + i] = subkeys[(15-r)*48 + i];
            }
        }
        desEncrypt(inBits, revSub, outBits);
    };

    /* ---------- 3DES (EDE) encryption of a 64‑bit block ---------- */
    auto tripleDesEncrypt = [&](int* blockBits, int* outBits){
        int step1[64], step2[64];
        desEncrypt(blockBits, subK1, step1);      // E with K1
        desDecrypt(step1,    subK2, step2);      // D with K2
        desEncrypt(step2,    subK3, outBits);    // E with K3
    };

    /* ---------- CFB mode (single 8‑byte segment) ---------- */
    int ivBits[64];
    bytesToBits(iv, ivBits);
    int plainBits[64];
    bytesToBits(plain, plainBits);
    int keystream[64];
    tripleDesEncrypt(ivBits, keystream);
    int cipherBits[64];
    for (int i = 0; i < 64; ++i) cipherBits[i] = plainBits[i] ^ keystream[i];
    int cipherBytes[8];
    bitsToBytes(cipherBits, cipherBytes);

    /* ---------- output ---------- */
    std::cout << "3DES‑CFB ciphertext (hex): ";
    for (int i = 0; i < 8; ++i){
        int hi = (cipherBytes[i] >> 4) & 0xF;
        int lo = cipherBytes[i] & 0xF;
        std::cout << (char)(hi < 10 ? hi + '0' : hi - 10 + 'A');
        std::cout << (char)(lo < 10 ? lo + '0' : lo - 10 + 'A');
    }
    std::cout << std::endl;

    /* ---------- clean up ---------- */
    delete[] IP; delete[] IPinv; delete[] PC1; delete[] PC2;
    delete[] ShiftSchedule; delete[] E; delete[] P;
    for (int i = 0; i < 8; ++i) delete[] S[i];
    delete[] subK1; delete[] subK2; delete[] subK3;

    return 0;
}
