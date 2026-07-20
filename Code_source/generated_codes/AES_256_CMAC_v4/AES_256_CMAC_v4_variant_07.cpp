/* LLM input variant 7: reverse-adversarial */
#include <iostream>
#include <iomanip>
#include <cstring>

#define BLOCKSIZE 16
#define KEYLEN    32
#define Rb        0x87   // reduction polynomial for subkey generation

/* ----------------------------------------------------------------
   Helper: rotate a 32‑bit word left by one byte (8 bits)
   ---------------------------------------------------------------- */
static int rotWord(int w0, int w1, int w2, int w3,
                   int &r0, int &r1, int &r2, int &r3)
{
    r0 = w1; r1 = w2; r2 = w3; r3 = w0;
}

/* ----------------------------------------------------------------
   AES class – encapsulates encryption and key schedule
   ---------------------------------------------------------------- */
class AES256
{
public:
    AES256(int *keyBytes)
    {
        roundKey = new int[60 * 4];          // 60 words (4‑byte each)
        keyExpansion(keyBytes);
    }

    ~AES256()
    {
        delete [] roundKey;
    }

    void encryptBlock(int *inBlk, int *outBlk)
    {
        int state[4][4];
        int i, j, r;

        /* copy input to state matrix (column‑major) */
        for (i = 0; i < 4; ++i)
            for (j = 0; j < 4; ++j)
                state[j][i] = inBlk[i * 4 + j] & 0xff;

        addRoundKey(state, 0);

        for (r = 1; r <= 13; ++r)
        {
            subBytes(state);
            shiftRows(state);
            mixColumns(state);
            addRoundKey(state, r);
        }

        subBytes(state);
        shiftRows(state);
        addRoundKey(state, 14);

        /* copy state matrix back to output */
        for (i = 0; i < 4; ++i)
            for (j = 0; j < 4; ++j)
                outBlk[i * 4 + j] = state[j][i] & 0xff;
    }

private:
    int *roundKey;   // 60 words (each word = 4 ints)

    /* ----------------------------------------------------------------
       Key expansion for 256‑bit key
       ---------------------------------------------------------------- */
    void keyExpansion(int *key)
    {
        int i, temp0, temp1, temp2, temp3;
        for (i = 0; i < 8; ++i)               // first 8 words are the key
        {
            roundKey[4 * i + 0] = key[4 * i + 0] & 0xff;
            roundKey[4 * i + 1] = key[4 * i + 1] & 0xff;
            roundKey[4 * i + 2] = key[4 * i + 2] & 0xff;
            roundKey[4 * i + 3] = key[4 * i + 3] & 0xff;
        }

        i = 8;
        while (i < 60)
        {
            temp0 = roundKey[4 * (i - 1) + 0];
            temp1 = roundKey[4 * (i - 1) + 1];
            temp2 = roundKey[4 * (i - 1) + 2];
            temp3 = roundKey[4 * (i - 1) + 3];

            if (i % 8 == 0)
            {
                /* RotWord */
                int r0, r1, r2, r3;
                rotWord(temp0, temp1, temp2, temp3, r0, r1, r2, r3);
                temp0 = r0; temp1 = r1; temp2 = r2; temp3 = r3;

                /* SubWord */
                temp0 = sbox(temp0); temp1 = sbox(temp1);
                temp2 = sbox(temp2); temp3 = sbox(temp3);

                /* Rcon */
                int rc = rcon(i / 8);
                temp0 ^= rc;
            }
            else if (i % 8 == 4)
            {
                temp0 = sbox(temp0); temp1 = sbox(temp1);
                temp2 = sbox(temp2); temp3 = sbox(temp3);
            }

            roundKey[4 * i + 0] = (roundKey[4 * (i - 8) + 0] ^ temp0) & 0xff;
            roundKey[4 * i + 1] = (roundKey[4 * (i - 8) + 1] ^ temp1) & 0xff;
            roundKey[4 * i + 2] = (roundKey[4 * (i - 8) + 2] ^ temp2) & 0xff;
            roundKey[4 * i + 3] = (roundKey[4 * (i - 8) + 3] ^ temp3) & 0xff;
            ++i;
        }
    }

    /* ----------------------------------------------------------------
       Core AES transformations (all use int, masked to 0‑255)
       ---------------------------------------------------------------- */
    static int sbox(int b)
    {
        static const int table[256] = {
            99,124,119,123,242,107,111,197,48,1,103,43,254,215,171,118,
            202,130,201,125,250,89,71,240,173,212,162,175,156,164,114,192,
            183,253,147,38,54,63,247,204,52,165,229,241,113,216,49,21,
            4,199,35,195,24,150,5,154,7,18,128,226,235,39,178,117,
            9,131,44,26,27,110,90,160,82,59,214,179,41,227,47,132,
            83,209,0,237,32,252,177,91,106,203,190,57,74,76,88,207,
            208,239,170,251,67,77,51,133,69,249,2,127,80,60,159,168,
            81,163,64,143,146,157,56,245,188,182,218,33,16,255,243,210,
            205,12,19,236,95,151,68,23,196,167,126,61,100,93,25,115,
            96,129,79,220,34,42,144,136,70,238,184,20,222,94,11,219,
            224,50,58,10,73,6,36,92,194,211,172,98,145,149,228,121,
            231,200,55,109,141,213,78,169,108,86,244,234,101,122,174,8,
            186,120,37,46,28,166,180,198,232,221,116,31,75,189,139,138,
            112,62,181,102,72,3,246,14,97,53,87,185,134,193,29,158,
            225,248,152,17,105,217,142,148,155,30,135,233,206,85,40,223,
            140,161,137,13,191,230,66,104,65,153,45,15,176,84,187,22
        };
        return table[b & 0xff];
    }

    static int rcon(int i)
    {
        static const int rc[15] = {
            0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36,
            0x6C,0xD8,0xAB,0x4D,0x9A
        };
        return rc[i-1];
    }

    static void subBytes(int state[4][4])
    {
        int i, j, tmp;
        for (i = 0; i < 4; ++i)
            for (j = 0; j < 4; ++j)
            {
                tmp = state[i][j];
                state[i][j] = sbox(tmp);
            }
    }

    static void shiftRows(int state[4][4])
    {
        int t;

        /* row 1 left 1 */
        t = state[1][0];
        state[1][0] = state[1][1];
        state[1][1] = state[1][2];
        state[1][2] = state[1][3];
        state[1][3] = t;

        /* row 2 left 2 */
        t = state[2][0];
        state[2][0] = state[2][2];
        state[2][2] = t;
        t = state[2][1];
        state[2][1] = state[2][3];
        state[2][3] = t;

        /* row 3 left 3 (right 1) */
        t = state[3][3];
        state[3][3] = state[3][2];
        state[3][2] = state[3][1];
        state[3][1] = state[3][0];
        state[3][0] = t;
    }

    static void mixColumns(int state[4][4])
    {
        int c, a0, a1, a2, a3, r0, r1, r2, r3;
        for (c = 0; c < 4; ++c)
        {
            a0 = state[0][c];
            a1 = state[1][c];
            a2 = state[2][c];
            a3 = state[3][c];

            r0 = xtime(a0) ^ xtime(a1) ^ a1 ^ a2 ^ a3;
            r1 = a0 ^ xtime(a1) ^ xtime(a2) ^ a2 ^ a3;
            r2 = a0 ^ a1 ^ xtime(a2) ^ xtime(a3) ^ a3;
            r3 = xtime(a0) ^ a0 ^ a1 ^ a2 ^ xtime(a3);

            state[0][c] = r0 & 0xff;
            state[1][c] = r1 & 0xff;
            state[2][c] = r2 & 0xff;
            state[3][c] = r3 & 0xff;
        }
    }

    static int xtime(int x)
    {
        int shifted = (x << 1) & 0xff;
        return (x & 0x80) ? (shifted ^ 0x1b) : shifted;
    }

    void addRoundKey(int state[4][4], int round)
    {
        int i, j, idx;
        for (i = 0; i < 4; ++i)
            for (j = 0; j < 4; ++j)
            {
                idx = round * 16 + i * 4 + j;
                state[j][i] ^= roundKey[idx] & 0xff;
            }
    }
};

/* ----------------------------------------------------------------
   CMAC helper functions (subkey generation, padding, XOR)
   ---------------------------------------------------------------- */
static void leftShiftOne(int *in, int *out)
{
    int i, carry = 0, nextCarry;
    for (i = BLOCKSIZE - 1; i >= 0; --i)
    {
        nextCarry = (in[i] & 0x80) ? 1 : 0;
        out[i] = ((in[i] << 1) & 0xff) | carry;
        carry = nextCarry;
    }
}

static void generateSubkeys(AES256 &aes, int *K1, int *K2)
{
    int zeroBlk[BLOCKSIZE];
    int L[BLOCKSIZE];
    int i;
    for (i = 0; i < BLOCKSIZE; ++i) zeroBlk[i] = 0;
    aes.encryptBlock(zeroBlk, L);

    leftShiftOne(L, K1);
    if (L[0] & 0x80) K1[BLOCKSIZE - 1] ^= Rb;

    leftShiftOne(K1, K2);
    if (K1[0] & 0x80) K2[BLOCKSIZE - 1] ^= Rb;
}

static void xorBlock(int *a, int *b, int *out)
{
    int i;
    for (i = 0; i < BLOCKSIZE; ++i)
        out[i] = (a[i] ^ b[i]) & 0xff;
}

/* ----------------------------------------------------------------
   CMAC calculation (message given as int array, length in bytes)
   ---------------------------------------------------------------- */
static void computeCmac(AES256 &aes,
                        int *msg, int msgLen,
                        int *tag)
{
    int K1[BLOCKSIZE];
    int K2[BLOCKSIZE];
    generateSubkeys(aes, K1, K2);

    int n = (msgLen + BLOCKSIZE - 1) / BLOCKSIZE;   // number of blocks
    int lastComplete = (msgLen % BLOCKSIZE == 0) ? 1 : 0;
    int i, j;

    int M_last[BLOCKSIZE];
    int M_i[BLOCKSIZE];
    int X[BLOCKSIZE];
    int Y[BLOCKSIZE];

    for (i = 0; i < BLOCKSIZE; ++i) X[i] = 0;

    for (i = 0; i < n - 1; ++i)
    {
        for (j = 0; j < BLOCKSIZE; ++j)
            M_i[j] = msg[i * BLOCKSIZE + j] & 0xff;

        xorBlock(X, M_i, Y);
        aes.encryptBlock(Y, X);
    }

    /* Build last block */
    if (lastComplete)
    {
        for (j = 0; j < BLOCKSIZE; ++j)
            M_last[j] = msg[(n - 1) * BLOCKSIZE + j] & 0xff;
        xorBlock(M_last, K1, M_last);
    }
    else
    {
        int paddingIdx = msgLen % BLOCKSIZE;
        for (j = 0; j < BLOCKSIZE; ++j)
            M_last[j] = 0;
        for (j = 0; j < paddingIdx; ++j)
            M_last[j] = msg[(n - 1) * BLOCKSIZE + j] & 0xff;
        M_last[paddingIdx] = 0x80;
        xorBlock(M_last, K2, M_last);
    }

    xorBlock(X, M_last, Y);
    aes.encryptBlock(Y, tag);
}

/* ----------------------------------------------------------------
   Test harness – deterministic key, message, and output
   ---------------------------------------------------------------- */
int main()
{
    /* deterministic 256‑bit key: reverse order 0x1F … 0x00 */
    int *key = new int[KEYLEN];
    for (int i = 0; i < KEYLEN; ++i) key[i] = (KEYLEN - 1 - i) & 0xff;

    /* deterministic adversarial message: 63 bytes descending from 0xFF */
    const int msgLen = 63;
    int *msg = new int[msgLen];
    for (int i = 0; i < msgLen; ++i) msg[i] = (0xFF - i) & 0xff;

    AES256 aes(key);
    int *tag = new int[BLOCKSIZE];

    computeCmac(aes, msg, msgLen, tag);

    /* print CMAC as hex */
    std::cout << "CMAC-256: ";
    for (int i = 0; i < BLOCKSIZE; ++i)
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (tag[i] & 0xff);
    std::cout << std::dec << std::endl;

    delete [] key;
    delete [] msg;
    delete [] tag;
    return 0;
}
