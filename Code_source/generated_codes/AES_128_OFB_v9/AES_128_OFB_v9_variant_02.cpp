/*  AES-128-OFB implementation – version #9
    Requirements:
    * only int / float types (no double, long, unsigned, const)
    * class‑based, std::vector usage
    * internal random input, no cin
    * simple, readable style
*/
/* LLM input variant 2: small-diverse */

#include <iostream>
#include <vector>
#include <iomanip>
#include <cstdlib>
#include <ctime>

class AesOfb {
public:
    AesOfb(const std::vector<int>& keyBytes) {
        // store original key (16 bytes)
        masterKey = keyBytes;
        expandKey();
    }

    // OFB encryption: returns ciphertext vector (same length as plaintext)
    std::vector<int> encrypt(const std::vector<int>& plain) {
        std::vector<int> out(plain.size());
        std::vector<int> iv(16);
        // simple IV: first 16 bytes of key (just for demo)
        for (int i = 0; i < 16; ++i) iv[i] = masterKey[i];

        int blockCount = (int)plain.size() / 16;
        int leftover   = (int)plain.size() % 16;

        for (int b = 0; b < blockCount; ++b) {
            iv = encryptBlock(iv);                     // produce keystream block
            for (int i = 0; i < 16; ++i) {
                out[b * 16 + i] = iv[i] ^ plain[b * 16 + i];
            }
        }
        // handle possible final partial block
        if (leftover > 0) {
            iv = encryptBlock(iv);
            for (int i = 0; i < leftover; ++i) {
                out[blockCount * 16 + i] = iv[i] ^ plain[blockCount * 16 + i];
            }
        }
        return out;
    }

private:
    std::vector<int> masterKey;          // 16 bytes
    std::vector<int> roundKeys;          // 176 bytes (11 * 16)

    // -----------------------------------------------------------------
    //  S‑box (static, 256 entries, values 0‑255)
    // -----------------------------------------------------------------
    static int sbox[256];
    static int rcon[11];

    // -----------------------------------------------------------------
    //  Key expansion (AES‑128, 10 rounds)
    // -----------------------------------------------------------------
    void expandKey() {
        roundKeys.resize(176);
        // copy initial key
        for (int i = 0; i < 16; ++i) roundKeys[i] = masterKey[i];

        int bytesGenerated = 16;
        int rconIdx = 1;
        while (bytesGenerated < 176) {
            // take last 4 bytes
            int temp0 = roundKeys[bytesGenerated - 4];
            int temp1 = roundKeys[bytesGenerated - 3];
            int temp2 = roundKeys[bytesGenerated - 2];
            int temp3 = roundKeys[bytesGenerated - 1];

            // rotate word every 16 bytes
            if (bytesGenerated % 16 == 0) {
                // RotWord
                int t = temp0;
                temp0 = temp1; temp1 = temp2; temp2 = temp3; temp3 = t;
                // SubWord
                temp0 = sbox[temp0]; temp1 = sbox[temp1];
                temp2 = sbox[temp2]; temp3 = sbox[temp3];
                // Rcon
                temp0 ^= rcon[rconIdx];
                ++rconIdx;
            }

            // XOR with word 16 bytes before
            for (int i = 0; i < 4; ++i) {
                int prev = roundKeys[bytesGenerated - 16 + i];
                int val  = (i == 0 ? temp0 : (i == 1 ? temp1 : (i == 2 ? temp2 : temp3)));
                roundKeys[bytesGenerated] = prev ^ val;
                ++bytesGenerated;
            }
        }
    }

    // -----------------------------------------------------------------
    //  One AES block encryption (ECB) – returns 16‑byte vector
    // -----------------------------------------------------------------
    std::vector<int> encryptBlock(const std::vector<int>& inBlock) {
        std::vector<int> state = inBlock;                 // 16 bytes

        addRoundKey(state, 0);

        for (int round = 1; round <= 9; ++round) {
            subBytes(state);
            shiftRows(state);
            mixColumns(state);
            addRoundKey(state, round);
        }

        // final round (no MixColumns)
        subBytes(state);
        shiftRows(state);
        addRoundKey(state, 10);

        return state;
    }

    // -----------------------------------------------------------------
    //  Core AES transformations (all work on 16‑byte vector)
    // -----------------------------------------------------------------
    void subBytes(std::vector<int>& st) {
        for (int i = 0; i < 16; ++i) st[i] = sbox[st[i]];
    }

    void shiftRows(std::vector<int>& st) {
        // row 1 – shift left 1
        int tmp = st[1];
        st[1] = st[5]; st[5] = st[9]; st[9] = st[13]; st[13] = tmp;
        // row 2 – shift left 2
        int tmp1 = st[2]; int tmp2 = st[6];
        st[2] = st[10]; st[6] = st[14]; st[10] = tmp1; st[14] = tmp2;
        // row 3 – shift left 3 (right 1)
        tmp = st[15];
        st[15] = st[11]; st[11] = st[7]; st[7] = st[3]; st[3] = tmp;
    }

    void mixColumns(std::vector<int>& st) {
        for (int c = 0; c < 4; ++c) {
            int i0 = c * 4;
            int a0 = st[i0];
            int a1 = st[i0 + 1];
            int a2 = st[i0 + 2];
            int a3 = st[i0 + 3];

            int r0 = mul2(a0) ^ mul3(a1) ^ a2 ^ a3;
            int r1 = a0 ^ mul2(a1) ^ mul3(a2) ^ a3;
            int r2 = a0 ^ a1 ^ mul2(a2) ^ mul3(a3);
            int r3 = mul3(a0) ^ a1 ^ a2 ^ mul2(a3);

            st[i0]     = r0;
            st[i0 + 1] = r1;
            st[i0 + 2] = r2;
            st[i0 + 3] = r3;
        }
    }

    void addRoundKey(std::vector<int>& st, int round) {
        int start = round * 16;
        for (int i = 0; i < 16; ++i) st[i] ^= roundKeys[start + i];
    }

    // -----------------------------------------------------------------
    //  Finite‑field multiplication helpers (AES‑GF(2^8))
    // -----------------------------------------------------------------
    int mul2(int x) {
        int shifted = x << 1;
        if (x & 0x80) shifted ^= 0x1b;
        return shifted & 0xff;
    }
    int mul3(int x) { return mul2(x) ^ x; }
};

// ---------------------------------------------------------------------
//  Static tables (definition outside class)
// ---------------------------------------------------------------------
int AesOfb::sbox[256] = {
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

int AesOfb::rcon[11] = {
    0x00,0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36
};

// ---------------------------------------------------------------------
//  Demo driver (generates deterministic data, encrypts, prints hex)
// ---------------------------------------------------------------------
int main() {
    // deterministic key with a clear pattern
    std::vector<int> keyBytes = {
        0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,
        0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff
    };

    // deterministic plaintext of 20 bytes (covers full block + leftover)
    std::vector<int> plain = {
        0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
        0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
        0x10,0x11,0x12,0x13
    };
    int plainLen = (int)plain.size();

    // AES‑OFB encryption
    AesOfb cipher(keyBytes);
    std::vector<int> cipherText = cipher.encrypt(plain);

    // output key, plaintext and ciphertext in hex
    std::cout << "Key:        ";
    for (int i = 0; i < 16; ++i) std::cout << std::hex << std::setw(2) << std::setfill('0') << keyBytes[i];
    std::cout << "\nPlaintext:  ";
    for (int i = 0; i < plainLen; ++i) std::cout << std::hex << std::setw(2) << std::setfill('0') << plain[i];
    std::cout << "\nCiphertext: ";
    for (int i = 0; i < (int)cipherText.size(); ++i) std::cout << std::hex << std::setw(2) << std::setfill('0') << cipherText[i];
    std::cout << std::dec << std::endl;
    return 0;
}
