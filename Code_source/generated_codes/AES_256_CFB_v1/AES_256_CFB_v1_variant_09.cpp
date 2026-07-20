#include <iostream>
#include <iomanip>
#include <cstdio>

/* LLM input variant 9: medium-deterministic-random */

//------------------------------------------------------------
//  AES-256 building blocks (int only, no const, no unsigned)
//------------------------------------------------------------

static int sbox[256] = {
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

static int rcon[15] = {
    1,2,4,8,16,32,64,128,27,54,108,216,171,77,154
};

//------------------------------------------------------------
//  Helper functions (split into temporaries)
//------------------------------------------------------------

void SubBytes(int st[4][4])
{
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            int idx = st[i][j];
            st[i][j] = sbox[idx];
        }
    }
}

void ShiftRows(int st[4][4])
{
    int t0 = st[1][0];
    for (int k = 0; k < 3; ++k) st[1][k] = st[1][k+1];
    st[1][3] = t0;

    int t1 = st[2][0];
    int t2 = st[2][1];
    st[2][0] = st[2][2];
    st[2][1] = st[2][3];
    st[2][2] = t1;
    st[2][3] = t2;

    int t3 = st[3][3];
    for (int k = 3; k > 0; --k) st[3][k] = st[3][k-1];
    st[3][0] = t3;
}

int xtime(int x)
{
    int shifted = x << 1;
    if (x & 0x80) shifted ^= 0x1b;
    return shifted & 0xff;
}

void MixColumns(int st[4][4])
{
    for (int c = 0; c < 4; ++c)
    {
        int a0 = st[0][c];
        int a1 = st[1][c];
        int a2 = st[2][c];
        int a3 = st[3][c];

        int b0 = xtime(a0) ^ (xtime(a1) ^ a1) ^ a2 ^ a3;
        int b1 = a0 ^ xtime(a1) ^ (xtime(a2) ^ a2) ^ a3;
        int b2 = a0 ^ a1 ^ xtime(a2) ^ (xtime(a3) ^ a3);
        int b3 = (xtime(a0) ^ a0) ^ a1 ^ a2 ^ xtime(a3);

        st[0][c] = b0 & 0xff;
        st[1][c] = b1 & 0xff;
        st[2][c] = b2 & 0xff;
        st[3][c] = b3 & 0xff;
    }
}

void AddRoundKey(int st[4][4], int rk[4][4])
{
    for (int c = 0; c < 4; ++c)
        for (int r = 0; r < 4; ++r)
            st[r][c] ^= rk[r][c];
}

//------------------------------------------------------------
//  Key schedule for 256‑bit key
//------------------------------------------------------------

void RotWord(int w[4])
{
    int tmp = w[0];
    w[0] = w[1];
    w[1] = w[2];
    w[2] = w[3];
    w[3] = tmp;
}

void SubWord(int w[4])
{
    for (int i = 0; i < 4; ++i)
        w[i] = sbox[w[i]];
}

void ExpandKey(int key[8][4], int rkeys[15][4][4])
{
    int w[60][4];
    for (int i = 0; i < 8; ++i)
        for (int j = 0; j < 4; ++j)
            w[i][j] = key[i][j];

    int i = 8;
    while (i < 60)
    {
        int temp[4];
        for (int j = 0; j < 4; ++j) temp[j] = w[i-1][j];

        if (i % 8 == 0)
        {
            RotWord(temp);
            SubWord(temp);
            temp[0] ^= rcon[(i/8)-1];
        }
        else if (i % 8 == 4)
        {
            SubWord(temp);
        }

        for (int j = 0; j < 4; ++j)
            w[i][j] = w[i-8][j] ^ temp[j];

        ++i;
    }

    for (int round = 0; round < 15; ++round)
    {
        for (int col = 0; col < 4; ++col)
            for (int row = 0; row < 4; ++row)
                rkeys[round][row][col] = w[round*4 + col][row];
    }
}

//------------------------------------------------------------
//  AES‑256 block encryption
//------------------------------------------------------------

void EncryptBlock(int block[4][4], int rkeys[15][4][4])
{
    AddRoundKey(block, rkeys[0]);

    for (int round = 1; round < 14; ++round)
    {
        SubBytes(block);
        ShiftRows(block);
        MixColumns(block);
        AddRoundKey(block, rkeys[round]);
    }

    SubBytes(block);
    ShiftRows(block);
    AddRoundKey(block, rkeys[14]);
}

//------------------------------------------------------------
//  CFB mode (encryption only)
//------------------------------------------------------------

void CfbEncrypt(int plain[2][4][4],
                int cipher[2][4][4],
                int iv[4][4],
                int rkeys[15][4][4])
{
    int feedback[4][4];
    for (int c = 0; c < 4; ++c)
        for (int r = 0; r < 4; ++r)
            feedback[r][c] = iv[r][c];

    for (int blk = 0; blk < 2; ++blk)
    {
        int encrypted[4][4];
        for (int c = 0; c < 4; ++c)
            for (int r = 0; r < 4; ++r)
                encrypted[r][c] = feedback[r][c];

        EncryptBlock(encrypted, rkeys);

        for (int c = 0; c < 4; ++c)
            for (int r = 0; r < 4; ++r)
                cipher[blk][r][c] = plain[blk][r][c] ^ encrypted[r][c];

        for (int c = 0; c < 4; ++c)
            for (int r = 0; r < 4; ++r)
                feedback[r][c] = cipher[blk][r][c];
    }
}

//------------------------------------------------------------
//  Helper to print 16‑byte blocks
//------------------------------------------------------------

void PrintBlock(const char* label, int blk[4][4])
{
    std::cout << label;
    for (int c = 0; c < 4; ++c)
        for (int r = 0; r < 4; ++r)
            std::cout << std::hex << std::setw(2) << std::setfill('0')
                      << (blk[r][c] & 0xff);
    std::cout << std::dec << "\n";
}

//------------------------------------------------------------
//  Simple deterministic pseudo‑random generator
//------------------------------------------------------------

int lcg_state = 0x1F2E3D4C;
int lcg()
{
    lcg_state = (lcg_state * 1103515245 + 12345) & 0x7fffffff;
    return (lcg_state >> 16) & 0xff;
}

//------------------------------------------------------------
//  Main – generate data, run CFB, display
//------------------------------------------------------------

int main()
{
    int key[8][4];
    int iv[4][4];
    int plain[2][4][4];
    int cipher[2][4][4];
    int roundKeys[15][4][4];

    // deterministic generation of key
    for (int i = 0; i < 8; ++i)
        for (int j = 0; j < 4; ++j)
            key[i][j] = lcg();

    // deterministic generation of IV
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            iv[i][j] = lcg();

    // deterministic generation of two plaintext blocks
    for (int b = 0; b < 2; ++b)
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                plain[b][i][j] = lcg();

    ExpandKey(key, roundKeys);
    CfbEncrypt(plain, cipher, iv, roundKeys);

    for (int i = 0; i < 2; ++i)
    {
        char lab[20];
        std::sprintf(lab, "Plain %d: ", i);
        PrintBlock(lab, plain[i]);

        std::sprintf(lab, "Cipher %d: ", i);
        PrintBlock(lab, cipher[i]);
    }

    return 0;
}
