/* LLM input variant 5: duplicate-heavy */
#include <iostream>
#include <iomanip>

// ---------- Helper Functions ----------
int xtime(int v) {
    int r = v << 1;
    if (r & 0x100) r ^= 0x11b;
    return r & 0xff;
}

int mul(int a, int b) {
    int p = 0;
    int i = 0;
    while (i < 8) {
        if (b & 1) p ^= a;
        int hi = a & 0x80;
        a <<= 1;
        if (hi) a ^= 0x11b;
        b >>= 1;
        i = i + 1;
    }
    return p & 0xff;
}

// ---------- AES Core ----------
void SubBytes(int s[4][4]) {
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
    int i = 0;
    while (i < 4) {
        int j = 0;
        while (j < 4) {
            s[i][j] = sbox[s[i][j]];
            j = j + 1;
        }
        i = i + 1;
    }
}

void ShiftRows(int s[4][4]) {
    // row 1
    int t = s[1][0];
    s[1][0] = s[1][1];
    s[1][1] = s[1][2];
    s[1][2] = s[1][3];
    s[1][3] = t;
    // row 2
    int t0 = s[2][0];
    int t1 = s[2][1];
    s[2][0] = s[2][2];
    s[2][1] = s[2][3];
    s[2][2] = t0;
    s[2][3] = t1;
    // row 3
    t = s[3][3];
    s[3][3] = s[3][2];
    s[3][2] = s[3][1];
    s[3][1] = s[3][0];
    s[3][0] = t;
}

void MixColumns(int s[4][4]) {
    int c = 0;
    while (c < 4) {
        int a0 = s[0][c];
        int a1 = s[1][c];
        int a2 = s[2][c];
        int a3 = s[3][c];

        int r0 = mul(a0,2) ^ mul(a1,3) ^ a2 ^ a3;
        int r1 = a0 ^ mul(a1,2) ^ mul(a2,3) ^ a3;
        int r2 = a0 ^ a1 ^ mul(a2,2) ^ mul(a3,3);
        int r3 = mul(a0,3) ^ a1 ^ a2 ^ mul(a3,2);

        s[0][c] = r0;
        s[1][c] = r1;
        s[2][c] = r2;
        s[3][c] = r3;

        c = c + 1;
    }
}

void AddRoundKey(int s[4][4], int rk[4][4]) {
    int i = 0;
    while (i < 4) {
        int j = 0;
        while (j < 4) {
            s[i][j] = s[i][j] ^ rk[i][j];
            j = j + 1;
        }
        i = i + 1;
    }
}

// ---------- Key Schedule ----------
void RotWord(int w[4]) {
    int tmp = w[0];
    w[0] = w[1];
    w[1] = w[2];
    w[2] = w[3];
    w[3] = tmp;
}

void SubWord(int w[4]) {
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
    int i = 0;
    while (i < 4) {
        w[i] = sbox[w[i]];
        i = i + 1;
    }
}

void KeyExpansion(int key[4][4], int roundKeys[11][4][4]) {
    int i = 0;
    while (i < 4) {
        int j = 0;
        while (j < 4) {
            roundKeys[0][j][i] = key[j][i];
            j = j + 1;
        }
        i = i + 1;
    }

    int rcon[11] = {0x00,0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36};

    int col = 4;
    while (col < 44) {
        int temp[4];
        int k = 0;
        while (k < 4) {
            temp[k] = roundKeys[(col-1)/4][k][(col-1)%4];
            k = k + 1;
        }

        if (col % 4 == 0) {
            RotWord(temp);
            SubWord(temp);
            temp[0] = temp[0] ^ rcon[col/4];
        }

        int row = 0;
        while (row < 4) {
            int prev = roundKeys[(col-4)/4][row][(col-4)%4];
            roundKeys[col/4][row][col%4] = prev ^ temp[row];
            row = row + 1;
        }
        col = col + 1;
    }
}

// ---------- Block Cipher ----------
void CipherBlock(int in[4][4], int out[4][4], int rks[11][4][4]) {
    int state[4][4];
    int i = 0;
    while (i < 4) {
        int j = 0;
        while (j < 4) {
            state[j][i] = in[j][i];
            j = j + 1;
        }
        i = i + 1;
    }

    AddRoundKey(state, rks[0]);

    int round = 1;
    while (round <= 9) {
        SubBytes(state);
        ShiftRows(state);
        MixColumns(state);
        AddRoundKey(state, rks[round]);
        round = round + 1;
    }

    SubBytes(state);
    ShiftRows(state);
    AddRoundKey(state, rks[10]);

    i = 0;
    while (i < 4) {
        int j = 0;
        while (j < 4) {
            out[j][i] = state[j][i];
            j = j + 1;
        }
        i = i + 1;
    }
}

// ---------- Counter ----------
void IncrementCtr(int ctr[4][4]) {
    int i = 3;
    while (i >= 0) {
        int j = 3;
        while (j >= 0) {
            ctr[j][i] = ctr[j][i] + 1;
            if (ctr[j][i] <= 0xff) {
                return;
            }
            ctr[j][i] = 0;
            j = j - 1;
        }
        i = i - 1;
    }
}

// ---------- Main ----------
int main() {
    // ----- deterministic test vector with many duplicates -----
    int key[4][4] = {
        {0xAA,0xAA,0xAA,0xAA},
        {0xAA,0xAA,0xAA,0xAA},
        {0xAA,0xAA,0xAA,0xAA},
        {0xAA,0xAA,0xAA,0xAA}
    };

    // plaintext (two identical blocks, all 'A')
    int plain[2][4][4] = {
        {
            { 'A','A','A','A' },
            { 'A','A','A','A' },
            { 'A','A','A','A' },
            { 'A','A','A','A' }
        },
        {
            { 'A','A','A','A' },
            { 'A','A','A','A' },
            { 'A','A','A','A' },
            { 'A','A','A','A' }
        }
    };

    // nonce / initial counter (all zeros)
    int ctr[4][4] = {
        {0x00,0x00,0x00,0x00},
        {0x00,0x00,0x00,0x00},
        {0x00,0x00,0x00,0x00},
        {0x00,0x00,0x00,0x00}
    };

    // expand key
    int roundKeys[11][4][4];
    KeyExpansion(key, roundKeys);

    // encrypt in CTR mode
    int cipher[2][4][4];
    int blockIdx = 0;
    while (blockIdx < 2) {
        int keystream[4][4];
        CipherBlock(ctr, keystream, roundKeys);

        int row = 0;
        while (row < 4) {
            int col = 0;
            while (col < 4) {
                cipher[blockIdx][row][col] = plain[blockIdx][row][col] ^ keystream[row][col];
                col = col + 1;
            }
            row = row + 1;
        }
        IncrementCtr(ctr);
        blockIdx = blockIdx + 1;
    }

    // ----- output -----
    std::cout << "Ciphertext (hex):" << std::endl;
    int b = 0;
    while (b < 2) {
        int r = 0;
        while (r < 4) {
            int c = 0;
            while (c < 4) {
                std::cout << std::hex << std::setw(2) << std::setfill('0')
                          << (cipher[b][r][c] & 0xff);
                c = c + 1;
            }
            r = r + 1;
        }
        std::cout << std::endl;
        b = b + 1;
    }
    return 0;
}
