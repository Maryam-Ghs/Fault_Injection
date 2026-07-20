#include <iostream>
#include <iomanip>

/* LLM input variant 1: minimal-boundary */

// ------------------------------------------------------------
// AES-256 constants (stored as signed int, values 0‑255)
// ------------------------------------------------------------
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
    1,2,4,8,16,32,64,128,27,54,108,216,171,77,154
};

// ------------------------------------------------------------
// Helper functions (verbose, step‑by‑step)
// ------------------------------------------------------------

// Multiply two bytes in GF(2^8) using the AES irreducible polynomial
int gmul(int a, int b) {
    int p = 0;
    for (int i = 0; i < 8; ++i) {
        if (b & 1) p ^= a;
        int hi = a & 0x80;
        a <<= 1;
        if (hi) a ^= 0x11b;
        b >>= 1;
    }
    return p & 0xff;
}

// SubBytes – apply the S‑box to every state byte
void SubBytes(int* st) {
    for (int i = 0; i < 16; ++i) {
        st[i] = sbox[st[i]];
    }
}

// ShiftRows – cyclically shift rows of the state
void ShiftRows(int* st) {
    int tmp = st[1];
    st[1]  = st[5];
    st[5]  = st[9];
    st[9]  = st[13];
    st[13] = tmp;

    int tmp1 = st[2];
    int tmp2 = st[6];
    st[2]  = st[10];
    st[6]  = st[14];
    st[10] = tmp1;
    st[14] = tmp2;

    tmp = st[15];
    st[15] = st[11];
    st[11] = st[7];
    st[7]  = st[3];
    st[3]  = tmp;
}

// MixColumns – mix each column using matrix multiplication
void MixColumns(int* st) {
    for (int c = 0; c < 4; ++c) {
        int i0 = c*4 + 0;
        int i1 = c*4 + 1;
        int i2 = c*4 + 2;
        int i3 = c*4 + 3;

        int a0 = st[i0];
        int a1 = st[i1];
        int a2 = st[i2];
        int a3 = st[i3];

        int r0 = gmul(a0,2) ^ gmul(a1,3) ^ a2 ^ a3;
        int r1 = a0 ^ gmul(a1,2) ^ gmul(a2,3) ^ a3;
        int r2 = a0 ^ a1 ^ gmul(a2,2) ^ gmul(a3,3);
        int r3 = gmul(a0,3) ^ a1 ^ a2 ^ gmul(a3,2);

        st[i0] = r0;
        st[i1] = r1;
        st[i2] = r2;
        st[i3] = r3;
    }
}

// AddRoundKey – XOR state with round key material
void AddRoundKey(int* st, int* rk) {
    for (int i = 0; i < 16; ++i) {
        st[i] ^= rk[i];
    }
}

// RotWord – rotate a 4‑byte word left by one byte
void RotWord(int* w) {
    int tmp = w[0];
    w[0] = w[1];
    w[1] = w[2];
    w[2] = w[3];
    w[3] = tmp;
}

// SubWord – apply S‑box to each byte of a word
void SubWord(int* w) {
    for (int i = 0; i < 4; ++i) {
        w[i] = sbox[w[i]];
    }
}

// KeyExpansion – produce round keys for AES‑256 (14 rounds + initial)
void KeyExpansion(int* key, int* w) {
    for (int i = 0; i < 8*4; ++i) {
        w[i] = key[i];
    }

    int bytesGenerated = 8*4;
    int rconIter = 0;
    int temp[4];

    while (bytesGenerated < (14+1)*4*4) {
        for (int i = 0; i < 4; ++i) {
            temp[i] = w[bytesGenerated - 4 + i];
        }

        if ((bytesGenerated / 4) % 8 == 0) {
            RotWord(temp);
            SubWord(temp);
            temp[0] ^= rcon[rc

onIter];
            ++rconIter;
        } else if ((bytesGenerated / 4) % 8 == 4) {
            SubWord(temp);
        }

        for (int i = 0; i < 4; ++i) {
            int prev = w[bytesGenerated - 8*4 + i];
            w[bytesGenerated + i] = prev ^ temp[i];
        }

        bytesGenerated += 4;
    }
}

// CipherBlock – encrypt a single 16‑byte block
void CipherBlock(int* inBlk, int* outBlk, int* roundKeys) {
    int* state = new int[16];
    for (int i = 0; i < 16; ++i) state[i] = inBlk[i];

    AddRoundKey(state, roundKeys);

    for (int round = 1; round <= 13; ++round) {
        SubBytes(state);
        ShiftRows(state);
        MixColumns(state);
        AddRoundKey(state, roundKeys + round*16);
    }

    SubBytes(state);
    ShiftRows(state);
    AddRoundKey(state, roundKeys + 14*16);

    for (int i = 0; i < 16; ++i) outBlk[i] = state[i];

    delete[] state;
}

// ------------------------------------------------------------
// CFB mode encryption (segment size = 128 bits)
// ------------------------------------------------------------
void CFBEncrypt(int* pt, int ptLen, int* key, int* iv, int* ct) {
    int* roundKeys = new int[(14+1)*16];
    KeyExpansion(key, roundKeys);

    int* feedback = new int[16];
    for (int i = 0; i < 16; ++i) feedback[i] = iv[i];

    int processed = 0;
    while (processed < ptLen) {
        int* encFeedback = new int[16];
        CipherBlock(feedback, encFeedback, roundKeys);

        int blockSize = 16;
        if (ptLen - processed < 16) blockSize = ptLen - processed;

        for (int i = 0; i < blockSize; ++i) {
            ct[processed + i] = pt[processed + i] ^ encFeedback[i];
        }

        for (int i = 0; i < 16; ++i) {
            if (i < blockSize) {
                feedback[i] = ct[processed + i];
            } else {
                feedback[i] = feedback[i];
            }
        }

        delete[] encFeedback;
        processed += blockSize;
    }

    delete[] feedback;
    delete[] roundKeys;
}

// ------------------------------------------------------------
// Utility: print a byte array as hex
// ------------------------------------------------------------
void PrintHex(const char* label, int* data, int len) {
    std::cout << label;
    for (int i = 0; i < len; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (data[i] & 0xff);
    }
    std::cout << std::dec << std::endl;
}

// ------------------------------------------------------------
// Main – generate minimal‑boundary inputs and run the algorithm
// ------------------------------------------------------------
int main() {
    // 1. Fixed 256‑bit key (32 bytes)
    int* key = new int[32];
    for (int i = 0; i < 32; ++i) key[i] = i;

    // 2. Fixed IV (16 bytes)
    int* iv = new int[16];
    for (int i = 0; i < 16; ++i) iv[i] = 0x00;

    // 3. Minimal‑boundary plaintexts
    // a) single byte
    int ptLenA = 1;
    int* ptA = new int[ptLenA];
    ptA[0] = 0x5a;

    // b) exactly one full block (16 bytes)
    int ptLenB = 16;
    int* ptB = new int[ptLenB];
    for (int i = 0; i < ptLenB; ++i) ptB[i] = i;

    // 4. Allocate ciphertext buffers
    int* ctA = new int[ptLenA];
    int* ctB = new int[ptLenB];

    // 5. Run CFB encryption for each case
    CFBEncrypt(ptA, ptLenA, key, iv, ctA);
    CFBEncrypt(ptB, ptLenB, key, iv, ctB);

    // 6. Print results
    PrintHex("Ciphertext (1‑byte) : ", ctA, ptLenA);
    PrintHex("Ciphertext (16‑bytes) : ", ctB, ptLenB);

    // 7. Clean up
    delete[] key;
    delete[] iv;
    delete[] ptA;
    delete[] ptB;
    delete[] ctA;
    delete[] ctB;

    return 0;
}
