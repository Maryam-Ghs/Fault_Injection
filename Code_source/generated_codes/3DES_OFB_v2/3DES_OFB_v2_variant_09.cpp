#include <iostream>
#include <iomanip>

/* LLM input variant 9: medium-deterministic-random */

/*--------------------------------------------------------------
  3‑DES in OFB mode – version #2
  * only int / float types
  * heap allocated buffers (new / delete)
  * class‑based, branch‑minimized, reordered arithmetic
--------------------------------------------------------------*/

static int IP[64] = {
    58,50,42,34,26,18,10,2 ,60,52,44,36,28,20,12,4 ,
    62,54,46,38,30,22,14,6 ,64,56,48,40,32,24,16,8 ,
    57,49,41,33,25,17,9 ,1 ,59,51,43,35,27,19,11,3 ,
    61,53,45,37,29,21,13,5 ,63,55,47,39,31,23,15,7
};

static int FP[64] = {
    40,8 ,48,16,56,24,64,32,39,7 ,47,15,55,23,63,31,
    38,6 ,46,14,54,22,62,30,37,5 ,45,13,53,21,61,29,
    36,4 ,44,12,52,20,60,28,35,3 ,43,11,51,19,59,27,
    34,2 ,42,10,50,18,58,26,33,1 ,41,9 ,49,17,57,25
};

static int PC1[56] = {
    57,49,41,33,25,17,9 ,1 ,58,50,42,34,26,18,
    10,2 ,59,51,43,35,27,19,11,3 ,60,52,44,36,
    63,55,47,39,31,23,15,7 ,62,54,46,38,30,22,
    14,6 ,61,53,45,37,29,21,13,5 ,28,20,12,4
};

static int PC2[48] = {
    14,17,11,24,1 ,5 ,3 ,28,15,6 ,21,10,
    23,19,12,4 ,26,8 ,16,7 ,27,20,13,2 ,
    41,52,31,37,47,55,30,40,51,45,33,48,
    44,49,39,56,34,53,46,42,50,36,29,32
};

static int SHIFTS[16] = {
    1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1
};

static int EBOX[48] = {
    32,1 ,2 ,3 ,4 ,5 ,4 ,5 ,6 ,7 ,8 ,9 ,
    8 ,9 ,10,11,12,13,12,13,14,15,16,17,
    16,17,18,19,20,21,20,21,22,23,24,25,
    24,25,26,27,28,29,28,29,30,31,32,1
};

static int SBOX[8][4][16] = {
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
      {6,11,13,8,1,4,10,7,9,5,0,15,14,2,3,12} },

    { {13,2,8,4,6,15,11,1,10,9,3,14,5,0,12,7},
      {1,15,13,8,10,3,7,4,12,5,6,11,0,14,9,2},
      {7,11,4,1,9,12,14,2,0,6,10,13,15,3,5,8},
      {2,1,14,7,4,10,8,13,15,12,9,0,3,5,6,11} }
};

static int PBOX[32] = {
    16,7 ,20,21,29,12,28,17,
    1 ,15,23,26,5 ,18,31,10,
    2 ,8 ,24,14,32,27,3 ,9 ,
    19,13,30,6 ,22,11,4 ,25
};

/*--------------------------------------------------------------
  Helper: generic permutation (input bits -> output bits)
--------------------------------------------------------------*/
void permute(const int *src, int *dst, const int *tbl, int n)
{
    for (int i = 0; i < n; ++i)
        dst[i] = src[tbl[i] - 1];
}

/*--------------------------------------------------------------
  Helper: left rotation of key halves (28‑bit each)
--------------------------------------------------------------*/
void rotate28(int *half, int steps)
{
    int tmp[28];
    for (int i = 0; i < 28; ++i)
        tmp[i] = half[(i + steps) % 28];
    for (int i = 0; i < 28; ++i)
        half[i] = tmp[i];
}

/*--------------------------------------------------------------
  Class: TripleDES in OFB mode
--------------------------------------------------------------*/
class TripleDES_OFB
{
    int *k1;   // 64‑bit
    int *k2;   // 64‑bit
    int *k3;   // 64‑bit
public:
    TripleDES_OFB(const int *keyA, const int *keyB, const int *keyC)
    {
        k1 = new int[64];
        k2 = new int[64];
        k3 = new int[64];
        for (int i = 0; i < 64; ++i) { k1[i] = keyA[i]; k2[i] = keyB[i]; k3[i] = keyC[i]; }
    }

    ~TripleDES_OFB()
    {
        delete [] k1; delete [] k2; delete [] k3;
    }

    /*----------------------------------------------------------
      Core DES encryption of a 64‑bit block with a 64‑bit key
    ----------------------------------------------------------*/
    void desEncrypt(const int *blk, const int *key, int *out) const
    {
        int ip[64]; permute(blk, ip, IP, 64);
        int left[32], right[32];
        for (int i = 0; i < 32; ++i) { left[i] = ip[i]; right[i] = ip[32 + i]; }

        /*--- key schedule ------------------------------------------------*/
        int pc1[56]; permute(key, pc1, PC1, 56);
        int c[28], d[28];
        for (int i = 0; i < 28; ++i) { c[i] = pc1[i]; d[i] = pc1[28 + i]; }

        for (int round = 0; round < 16; ++round)
        {
            rotate28(c, SHIFTS[round]);
            rotate28(d, SHIFTS[round]);
            int cd[56];
            for (int i = 0; i < 28; ++i) { cd[i] = c[i]; cd[28 + i] = d[i]; }
            int sub[48]; permute(cd, sub, PC2, 48);

            /*--- f‑function ------------------------------------------------*/
            int eout[48];
            permute(right, eout, EBOX, 48);
            for (int i = 0; i < 48; ++i) eout[i] ^= sub[i];          // XOR with subkey

            int sout[32];
            for (int s = 0; s < 8; ++s)
            {
                int row = (eout[s*6] << 1) + eout[s*6 + 5];
                int col = (eout[s*6 + 1] << 3) + (eout[s*6 + 2] << 2)
                        + (eout[s*6 + 3] << 1) + eout[s*6 + 4];
                int val = SBOX[s][row][col];
                for (int b = 0; b < 4; ++b)
                    sout[s*4 + b] = (val >> (3 - b)) & 1;
            }

            int fout[32];
            permute(sout, fout, PBOX, 32);

            int newRight[32];
            for (int i = 0; i < 32; ++i) newRight[i] = left[i] ^ fout[i];
            for (int i = 0; i < 32; ++i) left[i] = right[i];
            for (int i = 0; i < 32; ++i) right[i] = newRight[i];
        }

        int preFP[64];
        for (int i = 0; i < 32; ++i) { preFP[i] = right[i]; preFP[32 + i] = left[i]; }
        permute(preFP, out, FP, 64);
    }

    /*----------------------------------------------------------
      3‑DES: EDE (encrypt‑decrypt‑encrypt)
    ----------------------------------------------------------*/
    void tripleEncrypt(const int *blk, int *out) const
    {
        int tmp1[64]; desEncrypt(blk, k1, tmp1);
        int tmp2[64];
        desEncrypt(tmp1, k2, tmp2);
        desEncrypt(tmp2, k3, out);
    }

    /*----------------------------------------------------------
      OFB mode: generate keystream by encrypting IV repeatedly
    ----------------------------------------------------------*/
    void processOFB(const int *plain, int pLen, const int *iv, int *cipher) const
    {
        int *stream = new int[64];
        for (int i = 0; i < 64; ++i) stream[i] = iv[i];

        int blocks = pLen / 64;
        int pos = 0;
        while (blocks--)
        {
            int ks[64];
            tripleEncrypt(stream, ks);               // encrypt current IV
            for (int i = 0; i < 64; ++i)               // XOR with plaintext
                cipher[pos + i] = plain[pos + i] ^ ks[i];
            for (int i = 0; i < 64; ++i) stream[i] = ks[i]; // next IV = keystream
            pos += 64;
        }
        delete [] stream;
    }
};

/*--------------------------------------------------------------
  Convert byte array (0‑255) to bit array (0/1) and back
--------------------------------------------------------------*/
void bytesToBits(const int *bytes, int cnt, int *bits)
{
    int bitPos = 0;
    for (int i = 0; i < cnt; ++i)
    {
        int b = bytes[i];
        for (int j = 7; j >= 0; --j)
            bits[bitPos++] = (b >> j) & 1;
    }
}

void bitsToBytes(const int *bits, int cnt, int *bytes)
{
    int bytePos = 0;
    for (int i = 0; i < cnt; i += 8)
    {
        int val = 0;
        for (int j = 0; j < 8; ++j)
            val = (val << 1) + bits[i + j];
        bytes[bytePos++] = val;
    }
}

/*--------------------------------------------------------------
  Simple deterministic pseudo‑random byte generator (LCG)
--------------------------------------------------------------*/
void generateBytes(int *arr, int count, unsigned int seed)
{
    unsigned int state = seed;
    for (int i = 0; i < count; ++i)
    {
        state = (state * 1103515245u + 12345u) & 0x7fffffffu;
        arr[i] = (state >> 16) & 0xFF;
    }
}

/*--------------------------------------------------------------
  Main – generate deterministic test vectors, run 3‑DES‑OFB, print hex
--------------------------------------------------------------*/
int main()
{
    /*--- deterministic keys (8 bytes each) ----------------------*/
    int keyAbytes[8];
    int keyBbytes[8];
    int keyCbytes[8];
    generateBytes(keyAbytes, 8, 0x12345678u);
    generateBytes(keyBbytes, 8, 0x9abcdef0u);
    generateBytes(keyCbytes, 8, 0x0fedcba9u);

    int keyAbits[64], keyBbits[64], keyCbits[64];
    bytesToBits(keyAbytes, 8, keyAbits);
    bytesToBits(keyBbytes, 8, keyBbits);
    bytesToBits(keyCbytes, 8, keyCbits);

    TripleDES_OFB cipherEngine(keyAbits, keyBbits, keyCbits);

    /*--- plaintext: four blocks (32 bytes = 256 bits) -----------*/
    const int plainByteCount = 32;
    int plainBytes[plainByteCount];
    generateBytes(plainBytes, plainByteCount, 0xdeadbeefu);
    int plainBits[plainByteCount * 8];
    bytesToBits(plainBytes, plainByteCount, plainBits);

    /*--- IV (64‑bit) -------------------------------------------*/
    int ivBytes[8];
    generateBytes(ivBytes, 8, 0xfaceb00cu);
    int ivBits[64];
    bytesToBits(ivBytes, 8, ivBits);

    int cipherBits[plainByteCount * 8];
    cipherEngine.processOFB(plainBits, plainByteCount * 8, ivBits, cipherBits);

    int cipherBytes[plainByteCount];
    bitsToBytes(cipherBits, plainByteCount * 8, cipherBytes);

    std::cout << "Ciphertext (hex): ";
    for (int i = 0; i < plainByteCount; ++i)
        std::cout << std::hex << std::setw(2) << std::setfill('0') << cipherBytes[i];
    std::cout << std::dec << std::endl;

    return 0;
}
