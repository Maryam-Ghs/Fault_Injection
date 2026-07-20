/* LLM input variant 8: sparse-skewed */
// AES‑128‑CMAC implementation (version #1)
// ------------------------------------------------------------
// Only int (and float, though not used) are employed.
// No unsigned, long, double or const qualifiers.
// All data lives on the stack; the program generates its own
// test vector and prints the resulting CMAC in hex.
// ------------------------------------------------------------

#include <cstdio>
#include <cstring>

// -----------------------------------------------------------------
// 1.  AES building blocks (S‑box, Rcon)
// -----------------------------------------------------------------
int sbox[256] = {
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

int rcon[11] = {0,1,2,4,8,16,32,64,128,27,54};

// -----------------------------------------------------------------
// 2. Helper functions for low‑level AES operations
// -----------------------------------------------------------------
void subBytes(int state[4][4])
{
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            state[i][j] = sbox[state[i][j]];
}

void shiftRows(int state[4][4])
{
    // Row 1 – rotate left by 1
    int tmp = state[1][0];
    state[1][0] = state[1][1];
    state[1][1] = state[1][2];
    state[1][2] = state[1][3];
    state[1][3] = tmp;

    // Row 2 – rotate left by 2
    int tmp1 = state[2][0];
    int tmp2 = state[2][1];
    state[2][0] = state[2][2];
    state[2][1] = state[2][3];
    state[2][2] = tmp1;
    state[2][3] = tmp2;

    // Row 3 – rotate left by 3 (right by 1)
    tmp = state[3][3];
    state[3][3] = state[3][2];
    state[3][2] = state[3][1];
    state[3][1] = state[3][0];
    state[3][0] = tmp;
}

// xtime – multiply by {02} in GF(2^8)
int xtime(int x)
{
    int shifted = (x << 1) & 0xff;
    if (x & 0x80) shifted ^= 0x1b;
    return shifted;
}

void mixColumns(int state[4][4])
{
    for (int c = 0; c < 4; ++c)
    {
        int a0 = state[0][c];
        int a1 = state[1][c];
        int a2 = state[2][c];
        int a3 = state[3][c];

        int r0 = xtime(a0) ^ (xtime(a1) ^ a1) ^ a2 ^ a3;
        int r1 = a0 ^ xtime(a1) ^ (xtime(a2) ^ a2) ^ a3;
        int r2 = a0 ^ a1 ^ xtime(a2) ^ (xtime(a3) ^ a3);
        int r3 = (xtime(a0) ^ a0) ^ a1 ^ a2 ^ xtime(a3);

        state[0][c] = r0 & 0xff;
        state[1][c] = r1 & 0xff;
        state[2][c] = r2 & 0xff;
        state[3][c] = r3 & 0xff;
    }
}

void addRoundKey(int state[4][4], int roundKey[4][4])
{
    for (int r = 0; r < 4; ++r)
        for (int c = 0; c < 4; ++c)
            state[r][c] ^= roundKey[r][c];
}

// -----------------------------------------------------------------
// 3. Key expansion (produces 11 round keys)
// -----------------------------------------------------------------
void keyExpansion(const int key[16], int roundKeys[11][4][4])
{
    // First round key is the original key
    for (int i = 0; i < 16; ++i)
        roundKeys[0][i % 4][i / 4] = key[i];

    // Generate the remaining keys
    for (int r = 1; r <= 10; ++r)
    {
        // Temporary column
        int temp[4];
        for (int i = 0; i < 4; ++i)
            temp[i] = roundKeys[r-1][i][3];

        // RotWord
        int rot = temp[0];
        temp[0] = temp[1];
        temp[1] = temp[2];
        temp[2] = temp[3];
        temp[3] = rot;

        // SubWord
        for (int i = 0; i < 4; ++i)
            temp[i] = sbox[temp[i]];

        // Rcon
        temp[0] ^= rcon[r];

        // First column of new round key
        for (int i = 0; i < 4; ++i)
            roundKeys[r][i][0] = roundKeys[r-1][i][0] ^ temp[i];

        // Remaining columns
        for (int c = 1; c < 4; ++c)
            for (int i = 0; i < 4; ++i)
                roundKeys[r][i][c] = roundKeys[r-1][i][c] ^ roundKeys[r][i][c-1];
    }
}

// -----------------------------------------------------------------
// 4. AES‑128 block encryption (one 16‑byte block)
// -----------------------------------------------------------------
void aesEncryptBlock(const int in[16], int out[16], const int key[16])
{
    int state[4][4];
    // Copy input into state (column‑major)
    for (int i = 0; i < 16; ++i)
        state[i % 4][i / 4] = in[i];

    int roundKey[11][4][4];
    keyExpansion(key, roundKey);

    // Initial round
    addRoundKey(state, roundKey[0]);

    // 9 main rounds
    for (int r = 1; r <= 9; ++r)
    {
        subBytes(state);
        shiftRows(state);
        mixColumns(state);
        addRoundKey(state, roundKey[r]);
    }

    // Final round (no MixColumns)
    subBytes(state);
    shiftRows(state);
    addRoundKey(state, roundKey[10]);

    // Copy state back to output
    for (int i = 0; i < 16; ++i)
        out[i] = state[i % 4][i / 4];
}

// -----------------------------------------------------------------
// 5. CMAC helper utilities (left‑shift, xor, subkey generation)
// -----------------------------------------------------------------
void xorBlock(int a[16], const int b[16])
{
    for (int i = 0; i < 16; ++i)
        a[i] ^= b[i];
}

void leftShiftOne(int in[16])
{
    int carry = 0;
    for (int i = 15; i >= 0; --i)
    {
        int nextCarry = (in[i] & 0x80) ? 1 : 0;
        in[i] = ((in[i] << 1) & 0xff) | carry;
        carry = nextCarry;
    }
}

void generateSubkeys(const int key[16], int k1[16], int k2[16])
{
    int zeroBlock[16];
    for (int i = 0; i < 16; ++i) zeroBlock[i] = 0;

    int l[16];
    aesEncryptBlock(zeroBlock, l, key);

    // K1 = L << 1
    for (int i = 0; i < 16; ++i) k1[i] = l[i];
    leftShiftOne(k1);
    if (l[0] & 0x80) // if overflow, xor with Rb
        k1[15] ^= 0x87;

    // K2 = K1 << 1
    for (int i = 0; i < 16; ++i) k2[i] = k1[i];
    leftShiftOne(k2);
    if (k1[0] & 0x80)
        k2[15] ^= 0x87;
}

// -----------------------------------------------------------------
// 6. CMAC core algorithm
// -----------------------------------------------------------------
void computeCmac(const int key[16], const int msg[], int msgLen, int mac[16])
{
    int subK1[16], subK2[16];
    generateSubkeys(key, subK1, subK2);

    // Determine number of 16‑byte blocks (with padding)
    int nBlocks = (msgLen + 15) / 16;
    bool lastIsComplete = (msgLen % 16) == 0 && msgLen != 0;

    // Prepare last block
    int lastBlock[16];
    if (lastIsComplete)
    {
        // XOR with K1
        for (int i = 0; i < 16; ++i)
            lastBlock[i] = msg[(nBlocks-1)*16 + i] ^ subK1[i];
    }
    else
    {
        // Pad with 0x80 followed by zeros, then XOR with K2
        for (int i = 0; i < 16; ++i) lastBlock[i] = 0;
        int rem = msgLen % 16;
        for (int i = 0; i < rem; ++i)
            lastBlock[i] = msg[(nBlocks-1)*16 + i];
        lastBlock[rem] = 0x80;
        for (int i = 0; i < 16; ++i)
            lastBlock[i] ^= subK2[i];
    }

    // Initialize X = 0^128
    int x[16];
    for (int i = 0; i < 16; ++i) x[i] = 0;

    // Process all but last block
    for (int b = 0; b < nBlocks-1; ++b)
    {
        int y[16];
        for (int i = 0; i < 16; ++i)
            y[i] = msg[b*16 + i] ^ x[i];
        aesEncryptBlock(y, x, key);
    }

    // Process last block
    int yLast[16];
    for (int i = 0; i < 16; ++i)
        yLast[i] = lastBlock[i] ^ x[i];
    aesEncryptBlock(yLast, mac, key);
}

// -----------------------------------------------------------------
// 7. Demo in main(): sparse‑skewed message and key, print CMAC
// -----------------------------------------------------------------
int main()
{
    // Sparse 128‑bit key: only a few non‑zero bytes
    int key[16] = {
        0x01, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0xff
    };

    // Sparse, skewed message (48 bytes = 3 blocks) with most bytes zero
    int message[48] = {
        0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0x00, 0x00,

        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00
    };
    int msgLen = 48; // full length, multiple of block size

    int result[16];
    computeCmac(key, message, msgLen, result);

    // Print the 16‑byte MAC as hex
    std::printf("CMAC = ");
    for (int i = 0; i < 16; ++i)
        std::printf("%02x", result[i]);
    std::printf("\n");

    return 0;
}
