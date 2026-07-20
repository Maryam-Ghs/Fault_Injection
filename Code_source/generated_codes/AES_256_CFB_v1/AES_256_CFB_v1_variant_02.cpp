#include <iostream>
#include <iomanip>

/* LLM input variant 2: small-diverse */

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
    // row 1 left by 1
    int t0 = st[1][0];
    for (int k = 0; k < 3; ++k) st[1][k] = st[1][k+1];
    st[1][3] = t0;

    // row 2 left by 2
    int t1 = st[2][0];
    int t2 = st[2][1];
    st[2][0] = st[2][2];
    st[2][1] = st[2][3];
    st[2][2] = t1;
    st[2][3] = t2;

    // row 3 left by 3 (right by 1)
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

// produce 15 round keys (4×4 int each)
void ExpandKey(int key[8][4], int rkeys[15][4][4])
{
    // 60 words of 4 bytes each
    int w[60][4];

    // copy original key (8 words)
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

    // pack into round key matrices
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

    // final round (no MixColumns)
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
    // copy IV into feedback
    for (int c = 0; c < 4; ++c)
        for (int r = 0; r < 4; ++r)
            feedback[r][c] = iv[r][c];

    for (int blk = 0; blk < 2; ++blk)
    {
        int encrypted[4][4];
        // encrypt current feedback
        for (int c = 0; c < 4; ++c)
            for (int r = 0; r < 4; ++r)
                encrypted[r][c] = feedback[r][c];

        EncryptBlock(encrypted, rkeys);

        // XOR with plaintext to get ciphertext
        for (int c = 0; c < 4; ++c)
            for (int r = 0; r < 4; ++r)
                cipher[blk][r][c] = plain[blk][r][c] ^ encrypted[r][c];

        // next feedback = ciphertext block
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
//  Main – generate data, run CFB, display
//------------------------------------------------------------

int main()
{
    // 256‑bit key (8 × 4 bytes) – varied pattern
    int key[8][4] = {
        {0x00,0x11,0x22,0x33},
        {0x44,0x55,0x66,0x77},
        {0x88,0x99,0xaa,0xbb},
        {0xcc,0xdd,0xee,0xff},
        {0x10,0x32,0x54,0x76},
        {0x98,0xba,0xdc,0xfe},
        {0x01,0x23,0x45,0x67},
        {0x89,0xab,0xcd,0xef}
    };

    // 128‑bit IV – non‑sequential
    int iv[4][4] = {
        {0xf0,0xe1,0xd2,0xc3},
        {0xb4,0xa5,0x96,0x87},
        {0x78,0x69,0x5a,0x4b},
        {0x3c,0x2d,0x1e,0x0f}
    };

    // two plaintext blocks – mixed values
    int plain[2][4][4] = {
        {
            {0x00,0xff,0x01,0xfe},
            {0x02,0xfd,0x03,0xfc},
            {0x04,0xfb,0x05,0xfa},
            {0x06,0xf9,0x07,0xf8}
        },
        {
            {0x10,0x20,0x30,0x40},
            {0x50,0x60,0x70,0x80},
            {0x90,0xa0,0xb0,0xc0},
            {0xd0,0xe0,0xf0,0x00}
        }
    };

    int cipher[2][4][4];
    int roundKeys[15][4][4];

    ExpandKey(key, roundKeys);
    CfbEncrypt(plain, cipher, iv, roundKeys);

    // output
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
