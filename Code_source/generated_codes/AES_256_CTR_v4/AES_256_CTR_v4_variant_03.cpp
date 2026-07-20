#include <iostream>
#include <iomanip>

/* LLM input variant 3: zeros-and-ones */

// -------------------------------------------------------------------
//  AES‑256 in CTR mode – version #4
//  All data types are int (or float, not used here). No const, unsigned.
// -------------------------------------------------------------------

// S‑Box (static int, not const)
static int subBox[256] = {
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

// Rcon values (static int)
static int rCon[15] = {
    0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36,
    0x6C,0xD8,0xAB,0x4D,0x9A
};

// -------------------------------------------------------------------
// Helper functions for bytes manipulation (all int)
// -------------------------------------------------------------------
void copyWord(const int* src, int* dst) {
    int i = 0;
    while (i < 4) {
        dst[i] = src[i];
        i = i + 1;
    }
}

void xorWord(const int* a, const int* b, int* out) {
    int i = 0;
    while (i < 4) {
        out[i] = a[i] ^ b[i];
        i = i + 1;
    }
}

void rotWord(int* w) {
    int tmp = w[0];
    w[0] = w[1];
    w[1] = w[2];
    w[2] = w[3];
    w[3] = tmp;
}

void subWord(int* w) {
    int i = 0;
    while (i < 4) {
        w[i] = subBox[w[i]];
        i = i + 1;
    }
}

// GF(2^8) multiplication helpers
int mul2(int x) {
    int r = x << 1;
    if (r & 0x100) r = r ^ 0x11b;
    return r & 0xFF;
}
int mul3(int x) {
    return mul2(x) ^ x;
}

// -------------------------------------------------------------------
// AES‑256 class (heap allocated round keys, modular functions)
// -------------------------------------------------------------------
class Aes256Ctr {
    int* roundKeys;          // 15 * 16 bytes = 240 ints
    int* counterBlock;      // 16 bytes
public:
    // ctor: key (32 ints), iv (16 ints)
    Aes256Ctr(const int* keyBytes, const int* ivBytes) {
        // allocate round keys
        roundKeys = new int[16 * 15];
        // copy first two round keys directly from the key
        int i = 0;
        while (i < 32) {
            roundKeys[i] = keyBytes[i];
            i = i + 1;
        }
        // key expansion
        int words = 8;                     // 8 words (32 bytes) already present
        int totalWords = 4 * (14 + 1);     // 60 words for AES‑256
        int temp[4];
        while (words < totalWords) {
            // load previous word
            copyWord(&roundKeys[4 * (words - 1)], temp);
            if (words % 8 == 0) {
                rotWord(temp);
                subWord(temp);
                temp[0] = temp[0] ^ rCon[words / 8 - 1];
            } else if (words % 8 == 4) {
                subWord(temp);
            }
            // XOR with word 8 positions earlier
            int prev[4];
            copyWord(&roundKeys[4 * (words - 8)], prev);
            int newW[4];
            xorWord(prev, temp, newW);
            copyWord(newW, &roundKeys[4 * words]);
            words = words + 1;
        }
        // allocate counter block and copy iv
        counterBlock = new int[16];
        i = 0;
        while (i < 16) {
            counterBlock[i] = ivBytes[i];
            i = i + 1;
        }
    }

    // dtor
    ~Aes256Ctr() {
        delete[] roundKeys;
        delete[] counterBlock;
    }

    // ----------------------------------------------------------------
    // Core AES block encryption (standard direct computation)
    // ----------------------------------------------------------------
    void encryptBlock(const int* inBlock, int* outBlock) {
        int state[16];
        int i = 0;
        while (i < 16) {
            state[i] = inBlock[i] ^ roundKeys[i]; // AddRoundKey 0
            i = i + 1;
        }

        int round = 1;
        while (round < 14) {
            // SubBytes
            i = 0;
            while (i < 16) {
                state[i] = subBox[state[i]];
                i = i + 1;
            }
            // ShiftRows
            int tmp;
            // Row 1
            tmp = state[1];
            state[1]  = state[5];
            state[5]  = state[9];
            state[9]  = state[13];
            state[13] = tmp;
            // Row 2
            tmp = state[2];
            int tmp2 = state[6];
            state[2]  = state[10];
            state[6]  = state[14];
            state[10] = tmp;
            state[14] = tmp2;
            // Row 3
            tmp = state[3];
            state[3]  = state[15];
            state[15] = state[11];
            state[11] = state[7];
            state[7]  = tmp;
            // MixColumns
            i = 0;
            while (i < 4) {
                int col = i * 4;
                int a0 = state[col];
                int a1 = state[col + 1];
                int a2 = state[col + 2];
                int a3 = state[col + 3];
                int r0 = mul2(a0) ^ mul3(a1) ^ a2 ^ a3;
                int r1 = a0 ^ mul2(a1) ^ mul3(a2) ^ a3;
                int r2 = a0 ^ a1 ^ mul2(a2) ^ mul3(a3);
                int r3 = mul3(a0) ^ a1 ^ a2 ^ mul2(a3);
                state[col]     = r0;
                state[col + 1] = r1;
                state[col + 2] = r2;
                state[col + 3] = r3;
                i = i + 1;
            }
            // AddRoundKey
            i = 0;
            while (i < 16) {
                state[i] = state[i] ^ roundKeys[round * 16 + i];
                i = i + 1;
            }
            round = round + 1;
        }

        // Final round (no MixColumns)
        // SubBytes
        i = 0;
        while (i < 16) {
            state[i] = subBox[state[i]];
            i = i + 1;
        }
        // ShiftRows
        int tmp;
        // Row 1
        tmp = state[1];
        state[1]  = state[5];
        state[5]  = state[9];
        state[9]  = state[13];
        state[13] = tmp;
        // Row 2
        tmp = state[2];
        int tmp2 = state[6];
        state[2]  = state[10];
        state[6]  = state[14];
        state[10] = tmp;
        state[14] = tmp2;
        // Row 3
        tmp = state[3];
        state[3]  = state[15];
        state[15] = state[11];
        state[11] = state[7];
        state[7]  = tmp;
        // AddRoundKey
        i = 0;
        while (i < 16) {
            outBlock[i] = state[i] ^ roundKeys[14 * 16 + i];
            i = i + 1;
        }
    }

    // ----------------------------------------------------------------
    // Increment 128‑bit counter (big‑endian)
    // ----------------------------------------------------------------
    void incCounter() {
        int pos = 15;
        while (pos >= 0) {
            counterBlock[pos] = counterBlock[pos] + 1;
            if (counterBlock[pos] <= 0xFF) break;
            counterBlock[pos] = 0;
            pos = pos - 1;
        }
    }

    // ----------------------------------------------------------------
    // CTR encryption (in‑place, returns newly allocated ciphertext)
    // ----------------------------------------------------------------
    int* encryptCtr(const int* plain, int length) {
        int blocks = length / 16;
        int* cipher = new int[blocks * 16];
        int blk = 0;
        while (blk < blocks) {
            int keystream[16];
            encryptBlock(counterBlock, keystream);
            int i = 0;
            while (i < 16) {
                cipher[blk * 16 + i] = plain[blk * 16 + i] ^ keystream[i];
                i = i + 1;
            }
            incCounter();
            blk = blk + 1;
        }
        return cipher;
    }
};

// -------------------------------------------------------------------
// main – generate small predefined data, run AES‑256‑CTR, print hex
// -------------------------------------------------------------------
int main() {
    // 256‑bit key (32 ints) – alternating zeros and ones
    int key[32] = {
        0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,
        0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,
        0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,
        0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01
    };
    // 128‑bit IV / nonce (16 ints) – all zeros
    int iv[16] = {
        0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00
    };
    // Plaintext (two blocks = 32 ints) – mostly zeros with a few ones
    int plain[32] = {
        0x00,0x01,0x00,0x00,0x00,0x00,0x01,0x00,
        0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,
        0x01,0x00,0x00,0x00,0x00,0x01,0x00,0x00,
        0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00
    };

    // Create AES‑256‑CTR object
    Aes256Ctr aes(key, iv);
    // Encrypt
    int* cipher = aes.encryptCtr(plain, 32);

    // Print ciphertext as hex
    std::cout << "Ciphertext (hex):" << std::endl;
    int i = 0;
    while (i < 32) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (cipher[i] & 0xFF);
        if ((i + 1) % 16 == 0) std::cout << std::endl;
        else std::cout << " ";
        i = i + 1;
    }

    delete[] cipher;
    return 0;
}
