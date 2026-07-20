#include <iostream>

/* LLM input variant 7: reverse-adversarial */
//////////////////////////////////////////////////////////////
//  DES tables (int only, no const, no unsigned)            //
//////////////////////////////////////////////////////////////
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
    32,1,2,3,4,5, 4,5,6,7,8,9,
    8,9,10,11,12,13, 12,13,14,15,16,17,
    16,17,18,19,20,21, 20,21,22,23,24,25,
    24,25,26,27,28,29, 28,29,30,31,32,1};

int P[32] = {
    16,7,20,21,29,12,28,17,
    1,15,23,26,5,18,31,10,
    2,8,24,14,32,27,3,9,
    19,13,30,6,22,11,4,25};

int PC1[56] = {
    57,49,41,33,25,17,9, 1,58,50,42,34,26,18,
    10,2,59,51,43,35,27,19, 11,3,60,52,44,36,
    63,55,47,39,31,23,15, 7,62,54,46,38,30,22,
    14,6,61,53,45,37,29,21, 13,5,28,20,12,4};

int PC2[48] = {
    14,17,11,24,1,5, 3,28,15,6,21,10,
    23,19,12,4,26,8, 16,7,27,20,13,2,
    41,52,31,37,47,55, 30,40,51,45,33,48,
    44,49,39,56,34,53, 46,42,50,36,29,32};

int rotSchedule[16] = {
    1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1};

int S[8][64] = {
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

//////////////////////////////////////////////////////////////
//  Helper functions (all int based)                        //
//////////////////////////////////////////////////////////////
int rotL28(int v, int s)               // rotate 28‑bit value left
{
    int mask = (1 << 28) - 1;
    return ((v << s) & mask) | ((v >> (28 - s)) & mask);
}

// convert 8‑byte key to 56‑bit C/D halves (stored in int)
void splitKey(const int key[8], int &cHalf, int &dHalf)
{
    int tmp = 0;
    for (int i = 0; i < 56; ++i)
    {
        int bit = (key[PC1[i] / 8] >> (7 - (PC1[i] % 8))) & 1;
        tmp = (tmp << 1) | bit;
        if (i == 27) { cHalf = tmp; tmp = 0; }
        if (i == 55) { dHalf = tmp; }
    }
}

// produce 48‑bit subkey as two 24‑bit ints
void cookSubkey(int c, int d, int sub[2])
{
    int merged = (c << 28) | d;               // 56‑bit merged
    int left = 0, right = 0;
    for (int i = 0; i < 48; ++i)
    {
        int bit = (merged >> (56 - PC2[i])) & 1;
        if (i < 24) left = (left << 1) | bit;
        else        right = (right << 1) | bit;
    }
    sub[0] = left;
    sub[1] = right;
}

// generate 16 subkeys (each 48‑bit stored in two ints)
void forgeSubkeys(const int key[8], int subkeys[16][2])
{
    int c, d;
    splitKey(key, c, d);
    for (int r = 0; r < 16; )
    {
        c = rotL28(c, rotSchedule[r]);
        d = rotL28(d, rotSchedule[r]);
        cookSubkey(c, d, subkeys[r]);
        ++r;                         // while‑loop style change
    }
}

// expansion from 32‑bit to 48‑bit (as two 24‑bit ints)
void expand48(int r, int out[2])
{
    int left = 0, right = 0;
    for (int i = 0; i < 48; ++i)
    {
        int bit = (r >> (32 - E[i])) & 1;
        if (i < 24) left = (left << 1) | bit;
        else        right = (right << 1) | bit;
    }
    out[0] = left; out[1] = right;
}

// S‑box substitution (48‑bit -> 32‑bit)
int substitute(int in[2])
{
    int out = 0;
    for (int s = 0; s < 8; ++s)
    {
        int six = (s < 4) ?
            ((in[0] >> ((7 - s) * 6)) & 0x3F) :
            ((in[1] >> ((7 - s) * 6)) & 0x3F);
        int row = ((six & 0x20) >> 4) | (six & 0x01);
        int col = (six >> 1) & 0x0F;
        int val = S[s][row * 16 + col];
        out = (out << 4) | val;
    }
    return out;
}

// permutation P (32‑bit)
int permP(int x)
{
    int y = 0;
    for (int i = 0; i < 32; ++i)
    {
        int bit = (x >> (32 - P[i])) & 1;
        y = (y << 1) | bit;
    }
    return y;
}

// Feistel function F
int fFunc(int r, const int *sub)
{
    int exp[2];
    expand48(r, exp);
    exp[0] ^= sub[0];
    exp[1] ^= sub[1];
    int subd = substitute(exp);
    return permP(subd);
}

// single DES round (encrypt direction)
void roundEnc(int &l, int &r, const int *sub)
{
    int tmp = r;
    r = l ^ fFunc(r, sub);
    l = tmp;
}

// single DES round (decrypt direction)
void roundDec(int &l, int &r, const int *sub)
{
    int tmp = l;
    l = r ^ fFunc(l, sub);
    r = tmp;
}

// initial permutation
void applyIP(int &l, int &r)
{
    int block = (l << 32) | (r & 0xFFFFFFFF);
    int outL = 0, outR = 0;
    for (int i = 0; i < 64; ++i)
    {
        int srcPos = IP[i] - 1;
        int bit = (block >> (63 - srcPos)) & 1;
        if (i < 32) outL = (outL << 1) | bit;
        else        outR = (outR << 1) | bit;
    }
    l = outL; r = outR;
}

// final permutation
void applyFP(int &l, int &r)
{
    int block = (l << 32) | (r & 0xFFFFFFFF);
    int outL = 0, outR = 0;
    for (int i = 0; i < 64; ++i)
    {
        int srcPos = FP[i] - 1;
        int bit = (block >> (63 - srcPos)) & 1;
        if (i < 32) outL = (outL << 1) | bit;
        else        outR = (outR << 1) | bit;
    }
    l = outL; r = outR;
}

// DES encrypt 64‑bit block (two ints)
void desEncrypt(int &l, int &r, const int subkeys[16][2])
{
    applyIP(l, r);
    int i = 0;
    while (i < 16)
    {
        roundEnc(l, r, subkeys[i]);
        ++i;
    }
    // swap before final permutation
    int tmp = l; l = r; r = tmp;
    applyFP(l, r);
}

// DES decrypt 64‑bit block (two ints)
void desDecrypt(int &l, int &r, const int subkeys[16][2])
{
    applyIP(l, r);
    int i = 15;
    while (i >= 0)
    {
        roundDec(l, r, subkeys[i]);
        --i;
    }
    int tmp = l; l = r; r = tmp;
    applyFP(l, r);
}

// 3DES EDE (encrypt‑decrypt‑encrypt)
void tripleDES(int &L, int &R,
               const int subK1[16][2],
               const int subK2[16][2],
               const int subK3[16][2])
{
    desEncrypt(L, R, subK1);
    desDecrypt(L, R, subK2);
    desEncrypt(L, R, subK3);
}

//////////////////////////////////////////////////////////////
//  Main – deterministic reverse‑ordered data, run 3DES,     //
//  print results                                           //
//////////////////////////////////////////////////////////////
int main()
{
    // ---------- deterministic descending keys ----------
    int *keyA = new int[8];
    int *keyB = new int[8];
    int *keyC = new int[8];
    for (int i = 0; i < 8; ++i)
    {
        int val = 255 - i;            // descending from 255
        keyA[i] = val;
        keyB[i] = val;
        keyC[i] = val;
    }

    // ---------- derive subkeys for each key ----------
    int subA[16][2], subB[16][2], subC[16][2];
    forgeSubkeys(keyA, subA);
    forgeSubkeys(keyB, subB);
    forgeSubkeys(keyC, subC);

    // ---------- create a reverse‑ordered array of blocks ----------
    const int blocks = 7;                     // modest size, reverse order
    int *plainL = new int[blocks];
    int *plainR = new int[blocks];
    for (int i = 0; i < blocks; ++i)
    {
        plainL[i] = 0xFFFFFFFF - i;           // descending large values
        plainR[i] = 0xFFFFFFFF - (i * 2);
    }

    // ---------- encrypt with 3DES ----------
    int *cipherL = new int[blocks];
    int *cipherR = new int[blocks];
    int idx = blocks - 1;                     // process in reverse order
    while (idx >= 0)
    {
        int L = plainL[idx];
        int R = plainR[idx];
        tripleDES(L, R, subA, subB, subC);
        cipherL[idx] = L;
        cipherR[idx] = R;
        --idx;
    }

    // ---------- output ----------
    std::cout << "3DES‑EDE3 encryption results (hex):\n";
    for (int i = 0; i < blocks; ++i)
    {
        std::cout << "Block " << i << ": ";
        std::cout << std::hex << ((cipherL[i] >> 28) & 0xFFFFFFFF) << " ";
        std::cout << std::hex << ((cipherL[i] << 4) & 0xFFFFFFFF) << " ";
        std::cout << std::hex << ((cipherR[i] >> 28) & 0xFFFFFFFF) << " ";
        std::cout << std::hex << ((cipherR[i] << 4) & 0xFFFFFFFF) << "\n";
    }

    // ---------- cleanup ----------
    delete[] keyA; delete[] keyB; delete[] keyC;
    delete[] plainL; delete[] plainR;
    delete[] cipherL; delete[] cipherR;

    return 0;
}
