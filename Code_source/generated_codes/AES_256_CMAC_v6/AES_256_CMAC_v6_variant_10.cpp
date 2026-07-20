#include <iostream>
#include <vector>
#include <iomanip>
#include <string>

/* LLM input variant 10: large-safe-stress */

/* ====================  AES 256 Helper Tables  ==================== */
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

int rcon[15] = {
    0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36,0x6C,0xD8,0xAB,0x4D,0x9A
};

/* ====================  Utility Functions  ==================== */
std::vector<int> xorBlock(const std::vector<int>& a, const std::vector<int>& b) {
    std::vector<int> out(16);
    for (int i = 0; i < 16; ++i) out[i] = a[i] ^ b[i];
    return out;
}

std::vector<int> leftShiftOne(const std::vector<int>& in) {
    std::vector<int> out(16);
    int carry = 0;
    for (int i = 15; i >= 0; --i) {
        int newCarry = (in[i] & 0x80) ? 1 : 0;
        out[i] = ((in[i] << 1) & 0xFF) | carry;
        carry = newCarry;
    }
    return out;
}

/* ====================  AES Core  ==================== */
void subBytes(std::vector<int>& state) {
    for (int i = 0; i < 16; ++i) state[i] = sbox[state[i]];
}

void shiftRows(std::vector<int>& s) {
    int tmp;

    // Row 1 shift left 1
    tmp = s[1]; s[1] = s[5]; s[5] = s[9]; s[9] = s[13]; s[13] = tmp;

    // Row 2 shift left 2
    tmp = s[2]; s[2] = s[10]; s[10] = tmp;
    tmp = s[6]; s[6] = s[14]; s[14] = tmp;

    // Row 3 shift left 3 (right 1)
    tmp = s[15]; s[15] = s[11]; s[11] = s[7]; s[7] = s[3]; s[3] = tmp;
}

/* Galois multiplication by 2 */
int mul2(int x) {
    int r = x << 1;
    if (x & 0x80) r ^= 0x1B;
    return r & 0xFF;
}

/* Galois multiplication by 3 = mul2(x) ^ x */
int mul3(int x) {
    return mul2(x) ^ x;
}

void mixColumns(std::vector<int>& s) {
    for (int c = 0; c < 4; ++c) {
        int i0 = 4*c, i1 = i0+1, i2 = i0+2, i3 = i0+3;
        int a0 = s[i0], a1 = s[i1], a2 = s[i2], a3 = s[i3];
        s[i0] = mul2(a0) ^ mul3(a1) ^ a2 ^ a3;
        s[i1] = a0 ^ mul2(a1) ^ mul3(a2) ^ a3;
        s[i2] = a0 ^ a1 ^ mul2(a2) ^ mul3(a3);
        s[i3] = mul3(a0) ^ a1 ^ a2 ^ mul2(a3);
    }
}

void addRoundKey(std::vector<int>& state, const std::vector<int>& roundKey) {
    for (int i = 0; i < 16; ++i) state[i] ^= roundKey[i];
}

/* Key expansion for 256‑bit key (14 rounds + initial) */
std::vector< std::vector<int> > expandKey(const std::vector<int>& key) {
    std::vector< std::vector<int> > wk(15, std::vector<int>(16));
    // first two round keys are directly from the key
    for (int i = 0; i < 16; ++i) wk[0][i] = key[i];
    for (int i = 0; i < 16; ++i) wk[1][i] = key[16 + i];

    int rcIdx = 0;
    for (int i = 2; i < 15; ++i) {
        std::vector<int> temp = wk[i-1];
        // core every second round
        if (i % 2 == 0) {
            // rotate
            int t = temp[0];
            for (int j = 0; j < 15; ++j) temp[j] = temp[j+1];
            temp[15] = t;
            // subbytes
            for (int j = 0; j < 16; ++j) temp[j] = sbox[temp[j]];
            // rcon
            temp[0] ^= rcon[rcIdx++];
        }
        // xor with word 8 positions back
        for (int j = 0; j < 16; ++j) wk[i][j] = wk[i-2][j] ^ temp[j];
    }
    return wk;
}

/* One block AES encryption using pre‑expanded keys */
std::vector<int> aesEncrypt(const std::vector<int>& inBlock, const std::vector< std::vector<int> >& roundKeys) {
    std::vector<int> state = inBlock;

    // Initial AddRoundKey
    addRoundKey(state, roundKeys[0]);

    // ---- 13 full rounds (manual unrolling) ----
    // Round 1
    subBytes(state); shiftRows(state); mixColumns(state); addRoundKey(state, roundKeys[1]);
    // Round 2
    subBytes(state); shiftRows(state); mixColumns(state); addRoundKey(state, roundKeys[2]);
    // Round 3
    subBytes(state); shiftRows(state); mixColumns(state); addRoundKey(state, roundKeys[3]);
    // Round 4
    subBytes(state); shiftRows(state); mixColumns(state); addRoundKey(state, roundKeys[4]);
    // Round 5
    subBytes(state); shiftRows(state); mixColumns(state); addRoundKey(state, roundKeys[5]);
    // Round 6
    subBytes(state); shiftRows(state); mixColumns(state); addRoundKey(state, roundKeys[6]);
    // Round 7
    subBytes(state); shiftRows(state); mixColumns(state); addRoundKey(state, roundKeys[7]);
    // Round 8
    subBytes(state); shiftRows(state); mixColumns(state); addRoundKey(state, roundKeys[8]);
    // Round 9
    subBytes(state); shiftRows(state); mixColumns(state); addRoundKey(state, roundKeys[9]);
    // Round10
    subBytes(state); shiftRows(state); mixColumns(state); addRoundKey(state, roundKeys[10]);
    // Round11
    subBytes(state); shiftRows(state); mixColumns(state); addRoundKey(state, roundKeys[11]);
    // Round12
    subBytes(state); shiftRows(state); mixColumns(state); addRoundKey(state, roundKeys[12]);
    // Round13
    subBytes(state); shiftRows(state); mixColumns(state); addRoundKey(state, roundKeys[13]);

    // Final round (no MixColumns)
    subBytes(state); shiftRows(state); addRoundKey(state, roundKeys[14]);

    return state;
}

/* ====================  CMAC Core  ==================== */
std::vector<int> generateSubkey(const std::vector< std::vector<int> >& roundKeys) {
    std::vector<int> zeroBlock(16, 0);
    std::vector<int> L = aesEncrypt(zeroBlock, roundKeys);
    std::vector<int> K1 = leftShiftOne(L);
    if (L[0] & 0x80) {
        K1[15] ^= 0x87;
    }
    std::vector<int> K2 = leftShiftOne(K1);
    if (K1[0] & 0x80) {
        K2[15] ^= 0x87;
    }
    return K2; // return K2, caller will also have K1 if needed
}

std::vector<int> cmac(const std::vector<int>& key, const std::vector<int>& msg) {
    auto roundKeys = expandKey(key);
    // generate K1, K2
    std::vector<int> zeroBlock(16,0);
    std::vector<int> L = aesEncrypt(zeroBlock, roundKeys);
    std::vector<int> K1 = leftShiftOne(L);
    if (L[0] & 0x80) K1[15] ^= 0x87;
    std::vector<int> K2 = leftShiftOne(K1);
    if (K1[0] & 0x80) K2[15] ^= 0x87;

    // split message into 16‑byte blocks
    int nBlocks = (msg.size() + 15) / 16;
    bool lastComplete = (msg.size() % 16) == 0 && msg.size() != 0;
    std::vector<int> lastBlock(16, 0);
    if (lastComplete) {
        for (int i = 0; i < 16; ++i) lastBlock[i] = msg[(nBlocks-1)*16 + i] ^ K1[i];
    } else {
        // padding
        int rem = msg.size() % 16;
        for (int i = 0; i < rem; ++i) lastBlock[i] = msg[(nBlocks-1)*16 + i];
        lastBlock[rem] = 0x80;
        for (int i = 0; i < 16; ++i) lastBlock[i] ^= K2[i];
    }

    std::vector<int> X(16, 0);
    // process all but last block
    for (int b = 0; b < nBlocks-1; ++b) {
        std::vector<int> M(16);
        for (int i = 0; i < 16; ++i) M[i] = msg[b*16 + i];
        X = xorBlock(X, M);
        X = aesEncrypt(X, roundKeys);
    }
    // last block
    X = xorBlock(X, lastBlock);
    X = aesEncrypt(X, roundKeys);
    return X;
}

/* ====================  Main (deterministic test)  ==================== */
int main() {
    // 256‑bit key: 0x00 0x01 … 0x1F
    std::vector<int> key(32);
    for (int i = 0; i < 32; ++i) key[i] = i;

    // Large deterministic message: repeat a known phrase to reach ~16KB
    const std::string phrase = "The quick brown fox jumps over the lazy dog. ";
    const size_t repeatCount = 256; // 256 * 44 = 11264 bytes (~11KB)
    std::string txt;
    txt.reserve(phrase.size() * repeatCount);
    for (size_t i = 0; i < repeatCount; ++i) {
        txt += phrase;
    }

    std::vector<int> msg(txt.size());
    for (size_t i = 0; i < txt.size(); ++i) msg[i] = static_cast<int>(txt[i]);

    std::vector<int> tag = cmac(key, msg);

    std::cout << "CMAC‑256 = ";
    for (int i = 0; i < 16; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (tag[i] & 0xFF);
    }
    std::cout << std::dec << std::endl;
    return 0;
}
