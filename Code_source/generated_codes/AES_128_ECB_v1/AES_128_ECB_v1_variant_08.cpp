#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

/* LLM input variant 8: sparse-skewed */

class AES128_ECB {
public:
    AES128_ECB(const std::vector<int>& key) {
        // key schedule (16 bytes * (10 rounds + 1))
        roundKey.assign(176, 0);
        keyExpansion(key);
    }

    // encrypt a vector whose length is a multiple of 16 bytes
    void encrypt(std::vector<int>& data) {
        int blocks = data.size() / 16;
        for (int b = 0; b < blocks; ++b) {
            std::vector<int> block(16);
            for (int i = 0; i < 16; ++i)
                block[i] = data[b * 16 + i];
            encryptBlock(block);
            for (int i = 0; i < 16; ++i)
                data[b * 16 + i] = block[i];
        }
    }

private:
    std::vector<int> roundKey;               // 176 bytes = 11 round keys
    static int sbox[256];
    static int rcon[11];

    void keyExpansion(const std::vector<int>& key) {
        // first 16 bytes are the original key
        for (int i = 0; i < 16; ++i)
            roundKey[i] = key[i];

        int bytesGenerated = 16;
        int rconIter = 1;
        while (bytesGenerated < 176) {
            // take previous 4 bytes
            int temp0 = roundKey[bytesGenerated - 4];
            int temp1 = roundKey[bytesGenerated - 3];
            int temp2 = roundKey[bytesGenerated - 2];
            int temp3 = roundKey[bytesGenerated - 1];

            // rotate word every 16 bytes
            if (bytesGenerated % 16 == 0) {
                // RotWord
                int t = temp0;
                temp0 = temp1;
                temp1 = temp2;
                temp2 = temp3;
                temp3 = t;
                // SubWord
                temp0 = sbox[temp0 & 0xff];
                temp1 = sbox[temp1 & 0xff];
                temp2 = sbox[temp2 & 0xff];
                temp3 = sbox[temp3 & 0xff];
                // Rcon
                temp0 = temp0 ^ rcon[rconIter];
                ++rconIter;
            }

            // xor with word 16 bytes before
            roundKey[bytesGenerated]     = roundKey[bytesGenerated - 16] ^ temp0;
            roundKey[bytesGenerated + 1] = roundKey[bytesGenerated - 15] ^ temp1;
            roundKey[bytesGenerated + 2] = roundKey[bytesGenerated - 14] ^ temp2;
            roundKey[bytesGenerated + 3] = roundKey[bytesGenerated - 13] ^ temp3;
            bytesGenerated += 4;
        }
    }

    // -----------------------------------------------------------------
    // Core AES transformations – all loops are written explicitly.
    // -----------------------------------------------------------------
    void subBytes(std::vector<int>& state) {
        for (int i = 0; i < 16; ++i)
            state[i] = sbox[state[i] & 0xff];
    }

    void shiftRows(std::vector<int>& s) {
        // row 1 – shift left by 1
        int t = s[1];
        s[1]  = s[5];
        s[5]  = s[9];
        s[9]  = s[13];
        s[13] = t;

        // row 2 – shift left by 2 (swap pairs)
        t      = s[2];
        s[2]   = s[10];
        s[10]  = t;
        t      = s[6];
        s[6]   = s[14];
        s[14]  = t;

        // row 3 – shift left by 3 (right by 1)
        t      = s[15];
        s[15]  = s[11];
        s[11]  = s[7];
        s[7]   = s[3];
        s[3]   = t;
    }

    // multiplication in GF(2^8) by 2
    int xtime(int x) {
        int shifted = (x << 1) & 0xff;
        return (x & 0x80) ? (shifted ^ 0x1b) : shifted;
    }

    void mixColumns(std::vector<int>& s) {
        for (int c = 0; c < 4; ++c) {
            int i0 = c * 4;
            int a0 = s[i0];
            int a1 = s[i0 + 1];
            int a2 = s[i0 + 2];
            int a3 = s[i0 + 3];

            // calculate each new column value with reordered arithmetic
            int r0 = xtime(a0) ^ (xtime(a1) ^ a1) ^ a2 ^ a3;
            int r1 = a0 ^ xtime(a1) ^ (xtime(a2) ^ a2) ^ a3;
            int r2 = a0 ^ a1 ^ xtime(a2) ^ (xtime(a3) ^ a3);
            int r3 = (xtime(a0) ^ a0) ^ a1 ^ a2 ^ xtime(a3);

            s[i0]     = r0 & 0xff;
            s[i0 + 1] = r1 & 0xff;
            s[i0 + 2] = r2 & 0xff;
            s[i0 + 3] = r3 & 0xff;
        }
    }

    void addRoundKey(std::vector<int>& s, int round) {
        int start = round * 16;
        for (int i = 0; i < 16; ++i)
            s[i] = s[i] ^ roundKey[start + i];
    }

    void encryptBlock(std::vector<int>& block) {
        addRoundKey(block, 0);
        for (int r = 1; r <= 9; ++r) {
            subBytes(block);
            shiftRows(block);
            mixColumns(block);
            addRoundKey(block, r);
        }
        subBytes(block);
        shiftRows(block);
        addRoundKey(block, 10);
    }
};

// ---------------------------------------------------------------------
// Static tables (non‑const as required)
// ---------------------------------------------------------------------
int AES128_ECB::sbox[256] = {
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

int AES128_ECB::rcon[11] = {
  0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36,0x6C
};

int main() {
    // deterministic sparse-skewed key (mostly zeros)
    std::vector<int> key = {
        0x00,0x00,0x00,0x00,
        0x00,0x00,0x11,0x00,
        0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0xAA
    };

    // deterministic sparse-skewed plaintext (32 bytes, mostly zeros)
    std::vector<int> plain = {
        0x01,0x02,0x03,0x00,
        0x00,0x00,0x00,0x00,
        0x10,0x20,0x00,0x00,
        0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,
        0x00,0x00,0xFF,0x00,
        0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00
    };

    // show plaintext
    std::cout << "Plaintext : ";
    for (int i = 0; i < 32; ++i) {
        int v = plain[i] & 0xff;
        std::cout << std::hex << (v < 16 ? "0" : "") << v;
        if ((i + 1) % 16 == 0) std::cout << " ";
    }
    std::cout << std::dec << "\n";

    // encrypt
    AES128_ECB aes(key);
    aes.encrypt(plain);

    // show ciphertext
    std::cout << "Ciphertext: ";
    for (int i = 0; i < 32; ++i) {
        int v = plain[i] & 0xff;
        std::cout << std::hex << (v < 16 ? "0" : "") << v;
        if ((i + 1) % 16 == 0) std::cout << " ";
    }
    std::cout << std::dec << "\n";

    return 0;
}
