#include <iostream>
#include <iomanip>

/* LLM input variant 3: zeros-and-ones */

// ---------- S‑Box ----------
int sboxArr[256] = {
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

// ---------- GF(2^8) multiplication ----------
int xtime(int v) {
    int r = v << 1;
    if (r & 0x100) r ^= 0x11B;
    return r & 0xFF;
}

// ---------- Core AES transformations ----------
void subBytes(int st[4][4]) {
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j) {
            int idx = st[i][j];
            st[i][j] = sboxArr[idx];
        }
}

void shiftRows(int st[4][4]) {
    int t0, t1;
    // row 1 rotate left 1
    t0 = st[1][0];
    st[1][0] = st[1][1];
    st[1][1] = st[1][2];
    st[1][2] = st[1][3];
    st[1][3] = t0;
    // row 2 rotate left 2
    t0 = st[2][0];
    t1 = st[2][1];
    st[2][0] = st[2][2];
    st[2][1] = st[2][3];
    st[2][2] = t0;
    st[2][3] = t1;
    // row 3 rotate left 3 (right 1)
    t0 = st[3][3];
    st[3][3] = st[3][2];
    st[3][2] = st[3][1];
    st[3][1] = st[3][0];
    st[3][0] = t0;
}

void mixColumns(int st[4][4]) {
    for (int c = 0; c < 4; ++c) {
        int a0 = st[0][c];
        int a1 = st[1][c];
        int a2 = st[2][c];
        int a3 = st[3][c];
        int t0 = xtime(a0) ^ (xtime(a1) ^ a1) ^ a2 ^ a3;
        int t1 = a0 ^ xtime(a1) ^ (xtime(a2) ^ a2) ^ a3;
        int t2 = a0 ^ a1 ^ xtime(a2) ^ (xtime(a3) ^ a3);
        int t3 = (xtime(a0) ^ a0) ^ a1 ^ a2 ^ xtime(a3);
        st[0][c] = t0 & 0xFF;
        st[1][c] = t1 & 0xFF;
        st[2][c] = t2 & 0xFF;
        st[3][c] = t3 & 0xFF;
    }
}

void addRoundKey(int st[4][4], int rk[16]) {
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j) {
            int pos = i + 4 * j;
            st[i][j] ^= rk[pos];
        }
}

// ---------- Key schedule ----------
void subWord(int w[4]) {
    for (int i = 0; i < 4; ++i) w[i] = sboxArr[w[i]];
}

void rotWord(int w[4]) {
    int t = w[0];
    w[0] = w[1];
    w[1] = w[2];
    w[2] = w[3];
    w[3] = t;
}

void keyExpansion(const int key[32], int rkMat[15][16]) {
    int w[60][4];
    // copy initial key (8 words)
    for (int i = 0; i < 8; ++i)
        for (int j = 0; j < 4; ++j)
            w[i][j] = key[4 * i + j] & 0xFF;

    int rcon[15] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36,0x6C,0xD8,0xAB,0x4D,0x9A};

    for (int i = 8; i < 60; ++i) {
        int temp[4];
        for (int j = 0; j < 4; ++j) temp[j] = w[i-1][j];
        if (i % 8 == 0) {
            rotWord(temp);
            subWord(temp);
            temp[0] ^= rcon[i/8 - 1];
        } else if (i % 8 == 4) {
            subWord(temp);
        }
        for (int j = 0; j < 4; ++j)
            w[i][j] = (w[i-8][j] ^ temp[j]) & 0xFF;
    }

    // pack round keys (15 * 16 bytes)
    for (int r = 0; r < 15; ++r) {
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                int src = 4 * r + i;
                rkMat[r][4 * i + j] = w[src][j];
            }
        }
    }
}

// ---------- Single block AES ----------
void encryptBlock(const int in[16], int out[16], int rkMat[15][16]) {
    int state[4][4];
    // map input to state (column major)
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            state[i][j] = in[i + 4 * j] & 0xFF;

    addRoundKey(state, rkMat[0]);

    for (int round = 1; round < 14; ++round) {
        subBytes(state);
        shiftRows(state);
        mixColumns(state);
        addRoundKey(state, rkMat[round]);
    }

    subBytes(state);
    shiftRows(state);
    addRoundKey(state, rkMat[14]);

    // map state back to output
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            out[i + 4 * j] = state[i][j] & 0xFF;
}

// ---------- OCB (single‑block, no AAD) ----------
void ocbEncrypt(const int pt[16], const int key[32], const int nonce[16],
                int ct[16], int tg[16]) {
    int roundKeys[15][16];
    keyExpansion(key, roundKeys);

    int offset[16];
    encryptBlock(nonce, offset, roundKeys);

    int tmp[16];
    for (int i = 0; i < 16; ++i) tmp[i] = (pt[i] ^ offset[i]) & 0xFF;

    int encTmp[16];
    encryptBlock(tmp, encTmp, roundKeys);

    for (int i = 0; i < 16; ++i) ct[i] = (encTmp[i] ^ offset[i]) & 0xFF;

    int tagIn[16];
    for (int i = 0; i < 16; ++i) tagIn[i] = (offset[i] ^ pt[i]) & 0xFF;
    encryptBlock(tagIn, tg, roundKeys);
}

// ---------- Simple pseudo‑random generator ----------
int rngState = 0x12345678;
int rndInt() {
    rngState = (rngState * 1103515245 + 12345) & 0x7fffffff;
    return rngState & 0xFF;
}

// ---------- Main ----------
int main() {
    // Input variant emphasizing zeros and ones
    int key[32];
    int nonce[16];
    int plain[16];

    // Key: alternating 0 and 1
    for (int i = 0; i < 32; ++i) key[i] = (i % 2 == 0) ? 0x00 : 0x01;

    // Nonce: all zeros
    for (int i = 0; i < 16; ++i) nonce[i] = 0x00;

    // Plaintext: all zeros (identity-like)
    for (int i = 0; i < 16; ++i) plain[i] = 0x00;

    int cipher[16];
    int tag[16];
    ocbEncrypt(plain, key, nonce, cipher, tag);

    std::cout << "Plaintext : ";
    for (int i = 0; i < 16; ++i) std::cout << std::hex << std::setw(2) << std::setfill('0') << plain[i];
    std::cout << "\nCiphertext: ";
    for (int i = 0; i < 16; ++i) std::cout << std::hex << std::setw(2) << std::setfill('0') << cipher[i];
    std::cout << "\nTag       : ";
    for (int i = 0; i < 16; ++i) std::cout << std::hex << std::setw(2) << std::setfill('0') << tag[i];
    std::cout << std::dec << std::endl;
    return 0;
}
