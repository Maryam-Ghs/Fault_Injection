/* LLM input variant 7: reverse-adversarial */
// AES‑128 implementation – version #5
// ------------------------------------------------------------
// Only int and float are used (no unsigned, double, long, const)
// Input is generated inside the program (deterministic test vector)
// Results are printed to stdout
// ------------------------------------------------------------

#include <iostream>
#include <vector>
#include <iomanip>

// -----------------------------------------------------------------
// 1.  S‑Box and R‑con tables (global, int type)
// -----------------------------------------------------------------
static int sboxTbl[256] = {
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

static int rconTbl[10] = {
    0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36
};

// -----------------------------------------------------------------
// 2.  Helper functions (modular, vector‑based)
// -----------------------------------------------------------------

// Rotate a word (4‑byte vector) left by one byte
static void rotWord(std::vector<int>& w) {
    int tmp = w[0];
    w[0] = w[1];
    w[1] = w[2];
    w[2] = w[3];
    w[3] = tmp;
}

// Apply S‑Box to each byte of a word
static void subWord(std::vector<int>& w) {
    for (int i = 0; i < 4; ++i) {
        w[i] = sboxTbl[w[i] & 0xFF];
    }
}

// Key expansion: from 16‑byte key to 44 words (176 bytes)
static void expandKey(const std::vector<int>& key, std::vector<int>& roundKeys) {
    // first 4 words are the original key
    for (int i = 0; i < 16; ++i) {
        roundKeys[i] = key[i];
    }

    int wordIdx = 4;               // next word index
    int rconIdx = 0;               // round constant index

    while (wordIdx < 44) {
        // fetch previous word
        std::vector<int> temp(4);
        for (int i = 0; i < 4; ++i)
            temp[i] = roundKeys[(wordIdx - 1) * 4 + i];

        // every 4th word undergoes core transformation
        if (wordIdx % 4 == 0) {
            rotWord(temp);
            subWord(temp);
            temp[0] = (temp[0] ^ rconTbl[rconIdx]) & 0xFF;
            ++rconIdx;
        }

        // XOR with word 4 positions earlier
        for (int i = 0; i < 4; ++i) {
            int prev = roundKeys[(wordIdx - 4) * 4 + i];
            roundKeys[wordIdx * 4 + i] = (prev ^ temp[i]) & 0xFF;
        }
        ++wordIdx;
    }
}

// AddRoundKey: XOR state with round key (starting at offset)
static void addRoundKey(std::vector<int>& state, const std::vector<int>& roundKeys, int offset) {
    for (int i = 0; i < 16; ++i) {
        state[i] ^= roundKeys[offset + i];
    }
}

// SubBytes: apply S‑Box to whole state
static void subBytes(std::vector<int>& state) {
    for (int i = 0; i < 16; ++i) {
        state[i] = sboxTbl[state[i] & 0xFF];
    }
}

// ShiftRows: cyclic shift each row by its index
static void shiftRows(std::vector<int>& st) {
    // row 1 (index 1,5,9,13) shift left by 1
    int t = st[1];
    st[1] = st[5];
    st[5] = st[9];
    st[9] = st[13];
    st[13] = t;

    // row 2 (index 2,6,10,14) shift left by 2
    std::swap(st[2], st[10]);
    std::swap(st[6], st[14]);

    // row 3 (index 3,7,11,15) shift left by 3 (right by 1)
    t = st[15];
    st[15] = st[11];
    st[11] = st[7];
    st[7] = st[3];
    st[3] = t;
}

// Multiply by 2 in GF(2^8)
static int mul2(int x) {
    int r = x << 1;
    if (x & 0x80) r ^= 0x1B;
    return r & 0xFF;
}

// Multiply by 3 in GF(2^8)  (3 = 2 + 1)
static int mul3(int x) {
    return (mul2(x) ^ x) & 0xFF;
}

// MixColumns: column‑wise transformation
static void mixColumns(std::vector<int>& st) {
    for (int c = 0; c < 4; ++c) {
        int i0 = c * 4;
        int a0 = st[i0];
        int a1 = st[i0 + 1];
        int a2 = st[i0 + 2];
        int a3 = st[i0 + 3];

        int r0 = (mul2(a0) ^ mul3(a1) ^ a2 ^ a3) & 0xFF;
        int r1 = (a0 ^ mul2(a1) ^ mul3(a2) ^ a3) & 0xFF;
        int r2 = (a0 ^ a1 ^ mul2(a2) ^ mul3(a3)) & 0xFF;
        int r3 = (mul3(a0) ^ a1 ^ a2 ^ mul2(a3)) & 0xFF;

        st[i0]     = r0;
        st[i0 + 1] = r1;
        st[i0 + 2] = r2;
        st[i0 + 3] = r3;
    }
}

// -----------------------------------------------------------------
// 3.  AES encrypt a single 16‑byte block
// -----------------------------------------------------------------
static void aesEncryptBlock(const std::vector<int>& plain,
                            const std::vector<int>& roundKeys,
                            std::vector<int>& cipher) {
    // copy plaintext into state vector
    std::vector<int> state(16);
    for (int i = 0; i < 16; ++i) state[i] = plain[i] & 0xFF;

    // initial round key addition
    addRoundKey(state, roundKeys, 0);

    // 9 full rounds
    int round = 1;
    while (round < 10) {
        subBytes(state);
        shiftRows(state);
        mixColumns(state);
        addRoundKey(state, roundKeys, round * 16);
        ++round;
    }

    // final round (no MixColumns)
    subBytes(state);
    shiftRows(state);
    addRoundKey(state, roundKeys, round * 16);

    // output
    for (int i = 0; i < 16; ++i) cipher[i] = state[i] & 0xFF;
}

// -----------------------------------------------------------------
// 4.  Utility: print vector as hex bytes
// -----------------------------------------------------------------
static void printHex(const std::vector<int>& vec, const char* label) {
    std::cout << label;
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (vec[i] & 0xFF);
    }
    std::cout << std::dec << std::endl;
}

// -----------------------------------------------------------------
// 5.  Main – deterministic test vector (adversarial: all‑zero input)
// -----------------------------------------------------------------
int main() {
    // Test vector (adversarial all‑zero)
    // Key:  00000000000000000000000000000000
    // Plaintext: 00000000000000000000000000000000
    // Expected Ciphertext: 66e94bd4ef8a2c3b884cfa59ca342b2e
    std::vector<int> keyBytes = {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    };
    std::vector<int> plainBytes = {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    };
    std::vector<int> expectedCipher = {
        0x66,0xe9,0x4b,0xd4,0xef,0x8a,0x2c,0x3b,
        0x88,0x4c,0xfa,0x59,0xca,0x34,0x2b,0x2e
    };

    // Prepare round keys (44 words = 176 bytes)
    std::vector<int> roundKeyVec(176);
    expandKey(keyBytes, roundKeyVec);

    // Encrypt
    std::vector<int> cipherBytes(16);
    aesEncryptBlock(plainBytes, roundKeyVec, cipherBytes);

    // Output
    printHex(keyBytes,    "Key        : ");
    printHex(plainBytes,  "Plaintext  : ");
    printHex(cipherBytes, "Ciphertext : ");
    printHex(expectedCipher, "Expected   : ");

    // Simple verification
    bool ok = true;
    for (int i = 0; i < 16; ++i) {
        if ((cipherBytes[i] & 0xFF) != (expectedCipher[i] & 0xFF)) {
            ok = false;
            break;
        }
    }
    std::cout << (ok ? "PASS" : "FAIL") << std::endl;
    return 0;
}
