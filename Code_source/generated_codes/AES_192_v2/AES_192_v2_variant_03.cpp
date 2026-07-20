#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <ctime>

/* LLM input variant 3: zeros-and-ones */

//------------------------------------------------------------
//  Simple AES‑192 implementation (single 16‑byte block)
//  Uses only int and float types, stack arrays and helper
//  functions.  All data is generated inside the program.
//------------------------------------------------------------

// S‑box (filled with signed int values, masked to 0‑255 when used)
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

// Rcon for key expansion (only first 12 needed for AES‑192)
int rcon[12] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36,0x6C,0xD8};

//------------------------------------------------------------
// Helper: multiply by x in GF(2^8)
int xtime(int v) {
    int shifted = (v << 1) & 0xFF;
    return ((v & 0x80) ? (shifted ^ 0x1B) : shifted) & 0xFF;
}

//------------------------------------------------------------
// SubWord – apply S‑box to each byte of a 4‑byte word
void subWord(int src[4], int dst[4]) {
    int i = 0;
    while (i < 4) {
        dst[i] = sbox[src[i] & 0xFF] & 0xFF;
        ++i;
    }
}

//------------------------------------------------------------
// RotWord – rotate a word left by one byte
void rotWord(int src[4], int dst[4]) {
    int i = 0;
    while (i < 3) {
        dst[i] = src[i + 1];
        ++i;
    }
    dst[3] = src[0];
}

//------------------------------------------------------------
// XOR two words (in‑place)
void xorWord(int a[4], const int b[4]) {
    int i = 0;
    while (i < 4) {
        a[i] ^= b[i];
        ++i;
    }
}

//------------------------------------------------------------
// Expand a 24‑byte key into 13 round keys (each 16 bytes)
void expandKey(const int key[24], int roundKey[13][16]) {
    // w holds 52 words (4 bytes each)
    int w[52][4];
    int idx = 0;
    // copy original key (6 words)
    while (idx < 6) {
        w[idx][0] = key[4*idx];
        w[idx][1] = key[4*idx + 1];
        w[idx][2] = key[4*idx + 2];
        w[idx][3] = key[4*idx + 3];
        ++idx;
    }

    int i = 6;
    while (i < 52) {
        int temp[4];
        // copy previous word
        int j = 0;
        while (j < 4) {
            temp[j] = w[i - 1][j];
            ++j;
        }

        if (i % 6 == 0) {
            int rot[4];
            rotWord(temp, rot);
            subWord(rot, temp);
            temp[0] ^= rcon[(i/6)-1];
        } else if (i % 6 == 4) {
            subWord(temp, temp);
        }

        // w[i] = w[i-6] xor temp
        j = 0;
        while (j < 4) {
            w[i][j] = w[i - 6][j] ^ temp[j];
            ++j;
        }
        ++i;
    }

    // pack words into round keys (13 × 16 bytes)
    int r = 0;
    while (r < 13) {
        int c = 0;
        while (c < 4) {
            int wIdx = r * 4 + c;
            int b = 0;
            while (b < 4) {
                roundKey[r][c*4 + b] = w[wIdx][b];
                ++b;
            }
            ++c;
        }
        ++r;
    }
}

//------------------------------------------------------------
// AddRoundKey – XOR state with a round key
void addRoundKey(int state[4][4], const int *rk) {
    int col = 0;
    while (col < 4) {
        int row = 0;
        while (row < 4) {
            state[row][col] ^= rk[col*4 + row];
            ++row;
        }
        ++col;
    }
}

//------------------------------------------------------------
// SubBytes – apply S‑box to every byte in the state
void subBytes(int state[4][4]) {
    int c = 0;
    while (c < 4) {
        int r = 0;
        while (r < 4) {
            state[r][c] = sbox[state[r][c] & 0xFF] & 0xFF;
            ++r;
        }
        ++c;
    }
}

//------------------------------------------------------------
// ShiftRows – cyclic left shift of each row
void shiftRows(int state[4][4]) {
    // row 1 shift 1
    int tmp = state[1][0];
    state[1][0] = state[1][1];
    state[1][1] = state[1][2];
    state[1][2] = state[1][3];
    state[1][3] = tmp;

    // row 2 shift 2
    tmp = state[2][0];
    int tmp2 = state[2][1];
    state[2][0] = state[2][2];
    state[2][1] = state[2][3];
    state[2][2] = tmp;
    state[2][3] = tmp2;

    // row 3 shift 3 (right shift 1)
    tmp = state[3][3];
    state[3][3] = state[3][2];
    state[3][2] = state[3][1];
    state[3][1] = state[3][0];
    state[3][0] = tmp;
}

//------------------------------------------------------------
// MixColumns – column wise GF(2^8) multiplication
void mixColumns(int state[4][4]) {
    int c = 0;
    while (c < 4) {
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

        ++c;
    }
}

//------------------------------------------------------------
// Encrypt a single 16‑byte block with AES‑192
void encryptBlock(const int plain[16], const int key[24], int cipher[16]) {
    int st[4][4];   // state matrix

    // copy plaintext into state (column‑major)
    int i = 0;
    while (i < 16) {
        st[i % 4][i / 4] = plain[i] & 0xFF;
        ++i;
    }

    // key schedule
    int roundKey[13][16];
    expandKey(key, roundKey);

    // initial round
    addRoundKey(st, roundKey[0]);

    // 11 full rounds (Nr‑1 = 11)
    int r = 1;
    while (r < 12) {
        subBytes(st);
        shiftRows(st);
        mixColumns(st);
        addRoundKey(st, roundKey[r]);
        ++r;
    }

    // final round (no MixColumns)
    subBytes(st);
    shiftRows(st);
    addRoundKey(st, roundKey[12]);

    // copy state back to cipher array
    i = 0;
    while (i < 16) {
        cipher[i] = st[i % 4][i / 4] & 0xFF;
        ++i;
    }
}

//------------------------------------------------------------
// Helper: fill an array with deterministic zeros and ones
void deterministicBytes(int *buf, int len, bool startWithZero) {
    for (int i = 0; i < len; ++i) {
        buf[i] = ((i % 2) == (startWithZero ? 0 : 1)) ? 1 : 0;
    }
}

//------------------------------------------------------------
int main() {
    // deterministic "random" data emphasizing zeros and ones
    int plain[16];
    int key[24];

    // Plaintext: alternating 0,1 starting with 0
    deterministicBytes(plain, 16, true);
    // Key: first half zeros, second half ones
    for (int i = 0; i < 12; ++i) key[i] = 0;
    for (int i = 12; i < 24; ++i) key[i] = 1;

    // encrypt
    int cipher[16];
    encryptBlock(plain, key, cipher);

    // display
    std::cout << "Plaintext : ";
    for (int i = 0; i < 16; ++i)
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (plain[i] & 0xFF);
    std::cout << "\nKey       : ";
    for (int i = 0; i < 24; ++i)
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (key[i] & 0xFF);
    std::cout << "\nCiphertext: ";
    for (int i = 0; i < 16; ++i)
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (cipher[i] & 0xFF);
    std::cout << std::dec << std::endl;

    return 0;
}
