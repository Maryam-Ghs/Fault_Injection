#include <iostream>
#include <iomanip>

/* LLM input variant 9: medium-deterministic-random */

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
    // Deterministic pseudo‑random key (0x00 .. 0x1F)
    int key[32] = {
        0x00,0x01,0x02,0x03, 0x04,0x05,0x06,0x07,
        0x08,0x09,0x0A,0x0B, 0x0C,0x0D,0x0E,0x0F,
        0x10,0x11,0x12,0x13, 0x14,0x15,0x16,0x17,
        0x18,0x19,0x1A,0x1B, 0x1C,0x1D,0x1E,0x1F
    };

    // Deterministic pseudo‑random plaintext (0x00 .. 0x0F)
    int plain[16] = {
        0x00,0x01,0x02,0x03,
        0x04,0x05,0x06,0x07,
        0x08,0x09,0x0A,0x0B,
        0x0C,0x0D,0x0E,0x0F
    };

    // Simple identity S‑box (deterministic)
    int sbox[256];
    for (int i = 0; i < 256; ++i) sbox[i] = i;

    // Rcon (standard values)
    int rcon[15] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36,0x6C,0xD8,0xAB,0x4D,0x9A};

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
