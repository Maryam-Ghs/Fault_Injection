/* LLM input variant 4: signed-extremes */
// AES-128/ECB implementation – version #4
// Only int and float are used (no double, long, unsigned, const)
// All dynamic memory is handled with new/delete
// Input is generated inside the program, results are printed

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <ctime>

// ---------------------------------------------------------------------
// S‑Box (standard AES substitution box)
// ---------------------------------------------------------------------
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

// ---------------------------------------------------------------------
// Helper: multiply in GF(2^8) by 2 (xtime) – using only int arithmetic
// ---------------------------------------------------------------------
int xtime(int x) {
    int shifted = (x << 1) & 0xFF;
    int reduced = (x & 0x80) ? (shifted ^ 0x1B) : shifted;
    return reduced;
}

// ---------------------------------------------------------------------
// SubBytes – apply S‑Box to every byte of the state
// ---------------------------------------------------------------------
void SubBytes(int* st) {
    for (int i = 0; i < 16; ++i) {
        st[i] = sbox[st[i]];
    }
}

// ---------------------------------------------------------------------
// ShiftRows – cyclically shift rows of the 4×4 state matrix
// ---------------------------------------------------------------------
void ShiftRows(int* st) {
    // row 1 – shift 1 left
    int t1 = st[1]; st[1] = st[5]; st[5] = st[9]; st[9] = st[13]; st[13] = t1;
    // row 2 – shift 2 left
    int t2 = st[2]; int t6 = st[6];
    st[2] = st[10]; st[6] = st[14]; st[10] = t2; st[14] = t6;
    // row 3 – shift 3 left (or 1 right)
    int t3 = st[15];
    st[15] = st[11]; st[11] = st[7]; st[7] = st[3]; st[3] = t3;
}

// ---------------------------------------------------------------------
// MixColumns – mix each column of the state matrix
// ---------------------------------------------------------------------
void MixColumns(int* st) {
    for (int c = 0; c < 4; ++c) {
        int i0 = c * 4;
        int a0 = st[i0];
        int a1 = st[i0 + 1];
        int a2 = st[i0 + 2];
        int a3 = st[i0 + 3];
        int r0 = xtime(a0) ^ (xtime(a1) ^ a1) ^ a2 ^ a3;
        int r1 = a0 ^ xtime(a1) ^ (xtime(a2) ^ a2) ^ a3;
        int r2 = a0 ^ a1 ^ xtime(a2) ^ (xtime(a3) ^ a3);
        int r3 = (xtime(a0) ^ a0) ^ a1 ^ a2 ^ xtime(a3);
        st[i0]     = r0 & 0xFF;
        st[i0 + 1] = r1 & 0xFF;
        st[i0 + 2] = r2 & 0xFF;
        st[i0 + 3] = r3 & 0xFF;
    }
}

// ---------------------------------------------------------------------
// AddRoundKey – XOR state with round key
// ---------------------------------------------------------------------
void AddRoundKey(int* st, int* rkey) {
    for (int i = 0; i < 16; ++i) {
        st[i] ^= rkey[i];
    }
}

// ---------------------------------------------------------------------
// KeyExpansion – generate 11 round keys (each 16 bytes) from the cipher key
// ---------------------------------------------------------------------
void KeyExpansion(int* key, int* roundKeys) {
    // First round key is the original key
    for (int i = 0; i < 16; ++i) roundKeys[i] = key[i];

    // Rcon array (only first 10 needed)
    int rcon[10] = {1,2,4,8,16,32,64,128,27,54};

    // Generate remaining round keys
    int i = 16;
    while (i < 176) {
        int temp[4];
        // copy last word
        for (int j = 0; j < 4; ++j) temp[j] = roundKeys[i - 4 + j];

        // Every 16‑byte boundary apply core
        if ((i % 16) == 0) {
            // rotate left
            int t = temp[0];
            temp[0] = temp[1];
            temp[1] = temp[2];
            temp[2] = temp[3];
            temp[3] = t;
            // apply S‑Box
            for (int j = 0; j < 4; ++j) temp[j] = sbox[temp[j]];
            // XOR with Rcon
            temp[0] ^= rcon[(i / 16) - 1];
        }

        // XOR with word 16 bytes earlier
        for (int j = 0; j < 4; ++j) {
            roundKeys[i] = roundKeys[i - 16] ^ temp[j];
            ++i;
        }
    }
}

// ---------------------------------------------------------------------
// EncryptOneBlock – ECB encryption of a single 16‑byte block
// ---------------------------------------------------------------------
void EncryptOneBlock(int* block, int* roundKeys) {
    // Initial AddRoundKey
    AddRoundKey(block, roundKeys);

    // 9 main rounds
    int round = 1;
    while (round < 10) {
        SubBytes(block);
        ShiftRows(block);
        MixColumns(block);
        AddRoundKey(block, roundKeys + round * 16);
        ++round;
    }

    // Final round (no MixColumns)
    SubBytes(block);
    ShiftRows(block);
    AddRoundKey(block, roundKeys + 160); // round 10 key starts at offset 160
}

// ---------------------------------------------------------------------
// Utility: print a 16‑byte array as hex
// ---------------------------------------------------------------------
void PrintHex(const char* label, int* data) {
    std::cout << label;
    for (int i = 0; i < 16; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (data[i] & 0xFF);
    }
    std::cout << std::dec << std::endl;
}

// ---------------------------------------------------------------------
// Main – generate deterministic key & plaintext, encrypt, display
// ---------------------------------------------------------------------
int main() {
    std::srand((unsigned)std::time(0));

    // Allocate key, plaintext, ciphertext on heap
    int* key       = new int[16];
    int* plain     = new int[16];
    int* cipher    = new int[16];
    int* roundKeys = new int[176]; // 11 * 16 bytes

    // Deterministic test vectors mixing zero and positive values
    int fixed_key[16]   = {0, 1, 127, 128, 255, 34, 0, 200, 5, 250, 100, 0, 45, 90, 180, 255};
    int fixed_plain[16] = {255, 0, 50, 100, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 0};

    for (int i = 0; i < 16; ++i) {
        key[i]   = fixed_key[i];
        plain[i] = fixed_plain[i];
    }

    // Copy plaintext to cipher buffer (will be transformed in‑place)
    for (int i = 0; i < 16; ++i) cipher[i] = plain[i];

    // Key schedule
    KeyExpansion(key, roundKeys);

    // Encrypt the block
    EncryptOneBlock(cipher, roundKeys);

    // Output
    PrintHex("Key:        ", key);
    PrintHex("Plaintext:  ", plain);
    PrintHex("Ciphertext: ", cipher);

    // Release heap memory
    delete[] key;
    delete[] plain;
    delete[] cipher;
    delete[] roundKeys;

    return 0;
}
