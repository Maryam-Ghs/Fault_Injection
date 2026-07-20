/* LLM input variant 7: reverse-adversarial */
#include <iostream>
#include <iomanip>

void SubBytes(int state[4][4], const int sbox[256])
{
    for (int r = 0; r < 4; ++r)
        for (int c = 0; c < 4; ++c)
            state[r][c] = sbox[state[r][c]];
}

void ShiftRows(int state[4][4])
{
    int tmp;

    // row 1 left shift by 1
    tmp = state[1][0];
    for (int c = 0; c < 3; ++c) state[1][c] = state[1][c + 1];
    state[1][3] = tmp;

    // row 2 left shift by 2
    tmp = state[2][0];
    int tmp2 = state[2][1];
    state[2][0] = state[2][2];
    state[2][1] = state[2][3];
    state[2][2] = tmp;
    state[2][3] = tmp2;

    // row 3 left shift by 3 (right shift by 1)
    tmp = state[3][3];
    for (int c = 3; c > 0; --c) state[3][c] = state[3][c - 1];
    state[3][0] = tmp;
}

int xtime(int x)
{
    return ((x << 1) ^ ((x & 0x80) ? 0x1b : 0)) & 0xFF;
}

void MixColumns(int state[4][4])
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

        state[0][c] = r0 & 0xFF;
        state[1][c] = r1 & 0xFF;
        state[2][c] = r2 & 0xFF;
        state[3][c] = r3 & 0xFF;
    }
}

void AddRoundKey(int state[4][4], const int roundKey[4][4])
{
    for (int r = 0; r < 4; ++r)
        for (int c = 0; c < 4; ++c)
            state[r][c] ^= roundKey[r][c];
}

void KeyExpansion(const int key[32], int roundKeys[15][4][4],
                  const int sbox[256], const int rcon[15])
{
    // first 8 words (32 bytes) are the original key
    for (int i = 0; i < 8; ++i)
    {
        roundKeys[0][0][i % 4] = key[4 * i + 0];
        roundKeys[0][1][i % 4] = key[4 * i + 1];
        roundKeys[0][2][i % 4] = key[4 * i + 2];
        roundKeys[0][3][i % 4] = key[4 * i + 3];
    }

    int temp[4];
    int idx = 8; // next word index

    for (int round = 1; round <= 14; ++round)
    {
        // generate word idx
        // copy previous word
        for (int b = 0; b < 4; ++b) temp[b] = roundKeys[(idx - 1) / 4][b][(idx - 1) % 4];

        if (idx % 8 == 0)
        {
            // RotWord
            int t = temp[0];
            temp[0] = temp[1];
            temp[1] = temp[2];
            temp[2] = temp[3];
            temp[3] = t;
            // SubWord
            for (int b = 0; b < 4; ++b) temp[b] = sbox[temp[b]];
            // Rcon
            temp[0] ^= rcon[round];
        }
        else if (idx % 8 == 4)
        {
            for (int b = 0; b < 4; ++b) temp[b] = sbox[temp[b]];
        }

        // XOR with word idx-8
        for (int b = 0; b < 4; ++b)
        {
            int prev = roundKeys[(idx - 8) / 4][b][(idx - 8) % 4];
            temp[b] ^= prev;
        }

        // store new word
        roundKeys[idx / 4][0][idx % 4] = temp[0];
        roundKeys[idx / 4][1][idx % 4] = temp[1];
        roundKeys[idx / 4][2][idx % 4] = temp[2];
        roundKeys[idx / 4][3][idx % 4] = temp[3];

        ++idx;
    }
}

void EncryptBlock(const int in[16], int out[16],
                  const int roundKeys[15][4][4],
                  const int sbox[256])
{
    int state[4][4];
    // copy input to state (column major)
    for (int i = 0; i < 16; ++i)
        state[i % 4][i / 4] = in[i];

    AddRoundKey(state, roundKeys[0]);

    for (int round = 1; round < 14; ++round)
    {
        SubBytes(state, sbox);
        ShiftRows(state);
        MixColumns(state);
        AddRoundKey(state, roundKeys[round]);
    }

    // final round (no MixColumns)
    SubBytes(state, sbox);
    ShiftRows(state);
    AddRoundKey(state, roundKeys[14]);

    // copy state to output
    for (int i = 0; i < 16; ++i)
        out[i] = state[i % 4][i / 4];
}

int main()
{
    // AES‑256 test vector with reversed inputs (adversarial order)
    int key[32] = {
        0xf4,0xdf,0x14,0x09,
        0xa3,0x10,0x98,0x2d,
        0xd7,0x08,0x61,0x3b,
        0x07,0x2c,0x35,0x1f,
        0x81,0x77,0x7d,0x85,
        0xf0,0xae,0x73,0x2b,
        0xbe,0x71,0xca,0x15,
        0x10,0xeb,0x3d,0x60
    };

    int plain[16] = {
        0x2a,0x17,0x93,0x73,
        0x11,0x7e,0x3d,0xe9,
        0x96,0x9f,0x40,0x2e,
        0xe2,0xbe,0xc1,0x6b
    };

    // Reversed S‑box (descending order)
    int sbox[256];
    for (int i = 0; i < 256; ++i) sbox[i] = 255 - i;

    // Reversed Rcon
    int rcon[15] = {0x9A,0x4D,0xAB,0xD8,0x6C,0x36,0x1B,0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};

    int roundKeys[15][4][4];
    KeyExpansion(key, roundKeys, sbox, rcon);

    int cipher[16];
    EncryptBlock(plain, cipher, roundKeys, sbox);

    std::cout << "Ciphertext: ";
    for (int i = 0; i < 16; ++i)
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (cipher[i] & 0xFF);
    std::cout << std::dec << std::endl;

    return 0;
}
