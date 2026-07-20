/* LLM input variant 4: signed-extremes */
// AES-128/ECB implementation – version #6
// Only int and float are used (no double, long, unsigned, const)
// Input is generated inside the program, result is printed.

#include <iostream>
#include <vector>

// -------------------------------------------------------------------
// Helper: multiply by 2 in GF(2^8)
int xtime(int value) {
    int shifted = value << 1;
    if (shifted & 0x100) {               // if overflow beyond 8 bits
        shifted ^= 0x11b;                // reduce modulo the AES polynomial
    }
    return shifted & 0xff;               // keep only low 8 bits
}

// -------------------------------------------------------------------
// S‑Box (filled with the standard AES substitution values)
std::vector<int> sbox = {
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

// -------------------------------------------------------------------
// Rcon (round constants) – only first 10 needed for AES‑128
std::vector<int> rcon = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36};

// -------------------------------------------------------------------
// Key expansion: from a 16‑byte key produce 176 bytes (11 round keys)
std::vector<int> expandKey(const std::vector<int>& keyBlock) {
    std::vector<int> roundKeyStore(176);
    // first round key is the original key
    int idx = 0;
    while (idx < 16) {
        roundKeyStore[idx] = keyBlock[idx];
        ++idx;
    }

    int bytesGenerated = 16;
    int rconIdx = 0;
    std::vector<int> temp(4);

    while (bytesGenerated < 176) {
        // take last 4 bytes as temp
        for (int i = 0; i < 4; ++i) {
            temp[i] = roundKeyStore[bytesGenerated - 4 + i];
        }

        // every 16 bytes apply core schedule
        if ((bytesGenerated % 16) == 0) {
            // rotate left
            int first = temp[0];
            temp[0] = temp[1];
            temp[1] = temp[2];
            temp[2] = temp[3];
            temp[3] = first;
            // substitute through S‑Box
            for (int i = 0; i < 4; ++i) {
                temp[i] = sbox[temp[i]];
            }
            // xor with round constant
            temp[0] ^= rcon[rconIdx];
            ++rconIdx;
        }

        // generate next 4 bytes
        for (int i = 0; i < 4; ++i) {
            roundKeyStore[bytesGenerated] = roundKeyStore[bytesGenerated - 16] ^ temp[i];
            ++bytesGenerated;
        }
    }
    return roundKeyStore;
}

// -------------------------------------------------------------------
// AddRoundKey – XOR state with round key starting at offset 'off'
void addRoundKey(std::vector<int>& dataBlock, int off, const std::vector<int>& roundKeyStore) {
    int i = 0;
    while (i < 16) {
        dataBlock[i] ^= roundKeyStore[off + i];
        ++i;
    }
}

// -------------------------------------------------------------------
// SubBytes – apply S‑Box to each byte of the state
void subBytes(std::vector<int>& dataBlock) {
    int i = 0;
    while (i < 16) {
        dataBlock[i] = sbox[dataBlock[i]];
        ++i;
    }
}

// -------------------------------------------------------------------
// ShiftRows – cyclically shift rows in the 4×4 state matrix
void shiftRows(std::vector<int>& dataBlock) {
    // row 1 (index 1,5,9,13) shift left by 1
    int tmp = dataBlock[1];
    dataBlock[1]  = dataBlock[5];
    dataBlock[5]  = dataBlock[9];
    dataBlock[9]  = dataBlock[13];
    dataBlock[13] = tmp;

    // row 2 (index 2,6,10,14) shift left by 2
    int tmp1 = dataBlock[2];
    int tmp2 = dataBlock[6];
    dataBlock[2]  = dataBlock[10];
    dataBlock[6]  = dataBlock[14];
    dataBlock[10] = tmp1;
    dataBlock[14] = tmp2;

    // row 3 (index 3,7,11,15) shift left by 3 (right by 1)
    tmp = dataBlock[15];
    dataBlock[15] = dataBlock[11];
    dataBlock[11] = dataBlock[7];
    dataBlock[7]  = dataBlock[3];
    dataBlock[3]  = tmp;
}

// -------------------------------------------------------------------
// MixColumns – transform each column using finite‑field arithmetic
void mixColumns(std::vector<int>& dataBlock) {
    int col = 0;
    while (col < 4) {
        int i0 = col * 4;
        int i1 = i0 + 1;
        int i2 = i0 + 2;
        int i3 = i0 + 3;

        int a0 = dataBlock[i0];
        int a1 = dataBlock[i1];
        int a2 = dataBlock[i2];
        int a3 = dataBlock[i3];

        // pre‑compute multiplies by 2
        int b0 = xtime(a0);
        int b1 = xtime(a1);
        int b2 = xtime(a2);
        int b3 = xtime(a3);

        // mix column using the standard matrix
        dataBlock[i0] = b0 ^ (b1 ^ a1) ^ a2 ^ a3;
        dataBlock[i1] = a0 ^ b1 ^ (b2 ^ a2) ^ a3;
        dataBlock[i2] = a0 ^ a1 ^ b2 ^ (b3 ^ a3);
        dataBlock[i3] = (b0 ^ a0) ^ a1 ^ a2 ^ b3;

        ++col;
    }
}

// -------------------------------------------------------------------
// Encrypt a single 16‑byte block in ECB mode
std::vector<int> encryptBlock(const std::vector<int>& plainBlock,
                              const std::vector<int>& roundKeyStore) {
    std::vector<int> state = plainBlock;          // copy into mutable state

    // initial round
    addRoundKey(state, 0, roundKeyStore);

    // 9 main rounds
    int round = 1;
    while (round <= 9) {
        subBytes(state);
        shiftRows(state);
        mixColumns(state);
        addRoundKey(state, round * 16, roundKeyStore);
        ++round;
    }

    // final round (no MixColumns)
    subBytes(state);
    shiftRows(state);
    addRoundKey(state, 160, roundKeyStore);   // 10th round key starts at 160

    return state;
}

// -------------------------------------------------------------------
// Utility: print a vector of bytes as hex values
void printHex(const std::vector<int>& vec) {
    int i = 0;
    while (i < (int)vec.size()) {
        int v = vec[i];
        // ensure two‑digit hex
        if (v < 16) std::cout << '0';
        std::cout << std::hex << v;
        if (i != (int)vec.size() - 1) std::cout << ' ';
        ++i;
    }
    std::cout << std::dec << std::endl; // reset stream to decimal
}

// -------------------------------------------------------------------
int main() {
    // ----------------------------------------------------------------
    // Predefined 16‑byte key (mixed zero and extreme positive values)
    std::vector<int> keyBlock = {
        0x00,0xFF,0x7F,0x80,
        0x01,0x00,0xFF,0x80,
        0x55,0xAA,0x33,0x00,
        0xFF,0x7E,0x00,0x01
    };

    // Predefined 16‑byte plaintext (mixed zero, max, and mid values)
    std::vector<int> plainBlock = {
        0xFF,0x00,0xFF,0x00,
        0x00,0xFF,0x00,0xFF,
        0x7F,0x80,0x55,0xAA,
        0x33,0x00,0xFF,0x7E
    };

    // ----------------------------------------------------------------
    // Key schedule
    std::vector<int> roundKeyStore = expandKey(keyBlock);

    // Encryption
    std::vector<int> cipherBlock = encryptBlock(plainBlock, roundKeyStore);

    // Output
    std::cout << "Ciphertext (hex): ";
    printHex(cipherBlock);

    return 0;
}
