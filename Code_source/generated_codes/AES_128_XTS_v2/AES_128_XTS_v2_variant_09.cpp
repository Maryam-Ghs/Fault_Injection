#include <iostream>
#include <iomanip>

//------------------------------------------------------------
//  AES-128 building blocks (int only, no const, no unsigned)
//------------------------------------------------------------

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

void SubBytes(int state[16])
{
    // manual unrolling
    state[0] = sbox[state[0]];
    state[1] = sbox[state[1]];
    state[2] = sbox[state[2]];
    state[3] = sbox[state[3]];
    state[4] = sbox[state[4]];
    state[5] = sbox[state[5]];
    state[6] = sbox[state[6]];
    state[7] = sbox[state[7]];
    state[8] = sbox[state[8]];
    state[9] = sbox[state[9]];
    state[10] = sbox[state[10]];
    state[11] = sbox[state[11]];
    state[12] = sbox[state[12]];
    state[13] = sbox[state[13]];
    state[14] = sbox[state[14]];
    state[15] = sbox[state[15]];
}

void ShiftRows(int state[16])
{
    int t;

    // row 1 (shift left 1)
    t = state[1];
    state[1]  = state[5];
    state[5]  = state[9];
    state[9]  = state[13];
    state[13] = t;

    // row 2 (shift left 2)
    t = state[2];
    state[2]  = state[10];
    state[10] = t;
    t = state[6];
    state[6]  = state[14];
    state[14] = t;

    // row 3 (shift left 3)
    t = state[15];
    state[15] = state[11];
    state[11] = state[7];
    state[7]  = state[3];
    state[3]  = t;
}

// multiplication in GF(2^8) by 2
int xtime(int x)
{
    int r = x << 1;
    if (r & 0x100) r ^= 0x11b;
    return r & 0xff;
}

// MixColumns with full unrolling
void MixColumns(int state[16])
{
    int s0,s1,s2,s3;
    // column 0
    s0 = state[0]; s1 = state[1]; s2 = state[2]; s3 = state[3];
    state[0] = xtime(s0) ^ (xtime(s1) ^ s1) ^ s2 ^ s3;
    state[1] = s0 ^ xtime(s1) ^ (xtime(s2) ^ s2) ^ s3;
    state[2] = s0 ^ s1 ^ xtime(s2) ^ (xtime(s3) ^ s3);
    state[3] = (xtime(s0) ^ s0) ^ s1 ^ s2 ^ xtime(s3);

    // column 1
    s0 = state[4]; s1 = state[5]; s2 = state[6]; s3 = state[7];
    state[4] = xtime(s0) ^ (xtime(s1) ^ s1) ^ s2 ^ s3;
    state[5] = s0 ^ xtime(s1) ^ (xtime(s2) ^ s2) ^ s3;
    state[6] = s0 ^ s1 ^ xtime(s2) ^ (xtime(s3) ^ s3);
    state[7] = (xtime(s0) ^ s0) ^ s1 ^ s2 ^ xtime(s3);

    // column 2
    s0 = state[8]; s1 = state[9]; s2 = state[10]; s3 = state[11];
    state[8] = xtime(s0) ^ (xtime(s1) ^ s1) ^ s2 ^ s3;
    state[9] = s0 ^ xtime(s1) ^ (xtime(s2) ^ s2) ^ s3;
    state[10] = s0 ^ s1 ^ xtime(s2) ^ (xtime(s3) ^ s3);
    state[11] = (xtime(s0) ^ s0) ^ s1 ^ s2 ^ xtime(s3);

    // column 3
    s0 = state[12]; s1 = state[13]; s2 = state[14]; s3 = state[15];
    state[12] = xtime(s0) ^ (xtime(s1) ^ s1) ^ s2 ^ s3;
    state[13] = s0 ^ xtime(s1) ^ (xtime(s2) ^ s2) ^ s3;
    state[14] = s0 ^ s1 ^ xtime(s2) ^ (xtime(s3) ^ s3);
    state[15] = (xtime(s0) ^ s0) ^ s1 ^ s2 ^ xtime(s3);
}

void AddRoundKey(int state[16], int roundKey[16])
{
    // unrolled XOR
    state[0] ^= roundKey[0];
    state[1] ^= roundKey[1];
    state[2] ^= roundKey[2];
    state[3] ^= roundKey[3];
    state[4] ^= roundKey[4];
    state[5] ^= roundKey[5];
    state[6] ^= roundKey[6];
    state[7] ^= roundKey[7];
    state[8] ^= roundKey[8];
    state[9] ^= roundKey[9];
    state[10] ^= roundKey[10];
    state[11] ^= roundKey[11];
    state[12] ^= roundKey[12];
    state[13] ^= roundKey[13];
    state[14] ^= roundKey[14];
    state[15] ^= roundKey[15];
}

//------------------------------------------------------------
//  Key schedule (AES-128, 11 round keys)
//------------------------------------------------------------

int rcon[10] = {1,2,4,8,16,32,64,128,27,54};

void RotWord(int w[4])
{
    int t = w[0];
    w[0] = w[1];
    w[1] = w[2];
    w[2] = w[3];
    w[3] = t;
}

void SubWord(int w[4])
{
    w[0] = sbox[w[0]];
    w[1] = sbox[w[1]];
    w[2] = sbox[w[2]];
    w[3] = sbox[w[3]];
}

// roundKeys[round][byte]
void KeyExpansion(int key[16], int roundKeys[11][16])
{
    int i, j, temp[4];
    // copy original key as round 0
    for (i = 0; i < 16; ++i) roundKeys[0][i] = key[i];

    for (i = 1; i <= 10; ++i)
    {
        // first 4 bytes are special
        for (j = 0; j < 4; ++j) temp[j] = roundKeys[i-1][12 + j];
        RotWord(temp);
        SubWord(temp);
        temp[0] ^= rcon[i-1];
        for (j = 0; j < 4; ++j)
            roundKeys[i][j] = roundKeys[i-1][j] ^ temp[j];

        // remaining 12 bytes
        for (j = 4; j < 16; ++j)
            roundKeys[i][j] = roundKeys[i-1][j] ^ roundKeys[i][j-4];
    }
}

//------------------------------------------------------------
//  Single‑block AES encryption (ECB)
//------------------------------------------------------------

void AES_Encrypt_Block(int in[16], int out[16], int roundKeys[11][16])
{
    int state[16];
    int r;

    // copy input to state
    for (r = 0; r < 16; ++r) state[r] = in[r];

    AddRoundKey(state, roundKeys[0]);

    for (r = 1; r <= 9; ++r)
    {
        SubBytes(state);
        ShiftRows(state);
        MixColumns(state);
        AddRoundKey(state, roundKeys[r]);
    }

    // final round (no MixColumns)
    SubBytes(state);
    ShiftRows(state);
    AddRoundKey(state, roundKeys[10]);

    // copy out
    for (r = 0; r < 16; ++r) out[r] = state[r];
}

//------------------------------------------------------------
//  XTS helper: multiply tweak by α (GF(2^128) left shift)
//------------------------------------------------------------

void MultiplyByAlpha(int tweak[16])
{
    int carry = 0, nextCarry;
    int i;
    for (i = 15; i >= 0; --i)
    {
        nextCarry = (tweak[i] & 0x80) ? 1 : 0;      // high bit before shift
        tweak[i] = ((tweak[i] << 1) & 0xff) | carry;
        carry = nextCarry;
    }
    // if carry out, XOR with 0x87 (per XTS spec)
    if (carry)
        tweak[15] ^= 0x87;
}

//------------------------------------------------------------
//  XTS mode encryption (only full‑block case)
//------------------------------------------------------------

void XTS_Encrypt(int plaintext[], int pt_len,
                 int ciphertext[],
                 int data_key[16],
                 int tweak_key[16])
{
    int roundKeys_data[11][16];
    int roundKeys_tweak[11][16];
    int i, blockCount, blkIdx;
    int tweak[16];
    int encryptedTweak[16];
    int inBlock[16];
    int outBlock[16];
    int tmp[16];

    // key schedules for data and tweak
    KeyExpansion(data_key, roundKeys_data);
    KeyExpansion(tweak_key, roundKeys_tweak);

    // initial tweak = encrypt(IV) where IV = sector number (use zero)
    for (i = 0; i < 16; ++i) tweak[i] = 0;            // sector 0
    AES_Encrypt_Block(tweak, encryptedTweak, roundKeys_tweak);

    // copy encrypted tweak as first tweak value
    for (i = 0; i < 16; ++i) tweak[i] = encryptedTweak[i];

    blockCount = pt_len / 16;           // ignore any partial block (edge case)

    for (blkIdx = 0; blkIdx < blockCount; ++blkIdx)
    {
        // load plaintext block
        for (i = 0; i < 16; ++i) inBlock[i] = plaintext[blkIdx*16 + i];

        // XOR with tweak
        for (i = 0; i < 16; ++i) tmp[i] = inBlock[i] ^ tweak[i];

        // AES encrypt
        AES_Encrypt_Block(tmp, outBlock, roundKeys_data);

        // XOR again with tweak to get ciphertext block
        for (i = 0; i < 16; ++i) outBlock[i] ^= tweak[i];

        // store result
        for (i = 0; i < 16; ++i) ciphertext[blkIdx*16 + i] = outBlock[i];

        // update tweak for next block
        MultiplyByAlpha(tweak);
    }
}

//------------------------------------------------------------
//  Deterministic pseudo‑random generator for input data
//------------------------------------------------------------

void deterministic_fill(int *buf, int len)
{
    unsigned int state = 0xdeadbeefu; // fixed seed
    for (int i = 0; i < len; ++i)
    {
        state = state * 1664525u + 1013904223u;
        buf[i] = (state >> 24) & 0xff;
    }
}

/* LLM input variant 9: medium-deterministic-random */

//------------------------------------------------------------
//  Main – generate deterministic varied inputs, run XTS, print hex
//------------------------------------------------------------

int main()
{
    // -----------------------------------------------------------------
    // 1) Empty plaintext (edge case)
    // -----------------------------------------------------------------
    int pt_empty[0];
    int ct_empty[0];
    int data_key1[16] = {
        0x2b,0x7e,0x15,0x16,0x28,0xae,0xd2,0xa6,
        0xab,0xf7,0x15,0x88,0x09,0xcf,0x4f,0x3c
    };
    int tweak_key1[16] = {
        0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
        0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f
    };
    XTS_Encrypt(pt_empty, 0, ct_empty, data_key1, tweak_key1);
    std::cout << "Empty plaintext -> ciphertext length 0\n";

    // -----------------------------------------------------------------
    // 2) Single block – deterministic pseudo‑random bytes
    // -----------------------------------------------------------------
    int pt_one[16];
    int ct_one[16];
    deterministic_fill(pt_one, 16);
    XTS_Encrypt(pt_one, 16, ct_one, data_key1, tweak_key1);
    std::cout << "Single‑block (deterministic random) ciphertext:\n";
    for (int i = 0; i < 16; ++i)
        std::cout << std::hex << std::setw(2) << std::setfill('0') << ct_one[i];
    std::cout << "\n";

    // -----------------------------------------------------------------
    // 3) Ten blocks – deterministic pseudo‑random bytes
    // -----------------------------------------------------------------
    const int BLOCKS_TEN = 10;
    const int LEN_TEN = BLOCKS_TEN * 16;
    int pt_ten[LEN_TEN];
    int ct_ten[LEN_TEN];
    deterministic_fill(pt_ten, LEN_TEN);
    XTS_Encrypt(pt_ten, LEN_TEN, ct_ten, data_key1, tweak_key1);
    std::cout << "Ten‑block ciphertext (deterministic random):\n";
    for (int i = 0; i < LEN_TEN; ++i)
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << ct_ten[i];
        if ((i+1) % 16 == 0) std::cout << "\n";
    }

    // -----------------------------------------------------------------
    // 4) Twenty‑four blocks – larger deterministic random input
    // -----------------------------------------------------------------
    const int BLOCKS_24 = 24;
    const int LEN_24 = BLOCKS_24 * 16;
    int pt_24[LEN_24];
    int ct_24[LEN_24];
    deterministic_fill(pt_24, LEN_24);
    XTS_Encrypt(pt_24, LEN_24, ct_24, data_key1, tweak_key1);
    std::cout << "Twenty‑four‑block ciphertext (deterministic random):\n";
    for (int i = 0; i < LEN_24; ++i)
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << ct_24[i];
        if ((i+1) % 16 == 0) std::cout << "\n";
    }

    return 0;
}
