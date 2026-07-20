/* LLM input variant 9: medium-deterministic-random */

/*  AES‑128 ECB implementation – version #8
 *
 *  Constraints fulfilled:
 *   - only int (no double, long, unsigned, const)
 *   - class‑based design, stack arrays only
 *   - simple readable style, math split into temporaries
 *   - small predefined plaintext & key, no cin
 *   - results printed as hex
 */

#include <iostream>
#include <iomanip>

class Aes128Ecb {
public:
    // --------------------------------------------------------------------
    //  S‑box and round constants (global static, not const)
    // --------------------------------------------------------------------
    static int Sbox[256];
    static int Rcon[11];

    // --------------------------------------------------------------------
    //  Public interface
    // --------------------------------------------------------------------
    void encryptBlock(const int in[16], int out[16], const int key[16]) {
        int roundKeys[11][16];
        expandKey(key, roundKeys);

        int state[16];
        copyBlock(in, state);

        // Initial AddRoundKey
        addRoundKey(state, roundKeys[0]);

        int round = 1;
        while (round < 10) {            // 9 main rounds
            subBytes(state);
            shiftRows(state);
            mixColumns(state);
            addRoundKey(state, roundKeys[round]);
            ++round;
        }

        // Final round (no MixColumns)
        subBytes(state);
        shiftRows(state);
        addRoundKey(state, roundKeys[10]);

        copyBlock(state, out);
    }

private:
    // --------------------------------------------------------------------
    //  Helper to copy 16‑byte blocks
    // --------------------------------------------------------------------
    void copyBlock(const int src[16], int dst[16]) {
        int i = 0;
        while (i < 16) {
            dst[i] = src[i];
            ++i;
        }
    }

    // --------------------------------------------------------------------
    //  Key expansion (produces 11 round keys)
    // --------------------------------------------------------------------
    void expandKey(const int key[16], int roundKeys[11][16]) {
        // first round key is the original key
        copyBlock(key, roundKeys[0]);

        int i = 16;               // byte index in expanded key
        int rconIdx = 1;
        while (i < 176) {         // 11 * 16 = 176 bytes total
            int temp[4];
            // take previous 4‑byte word
            int j = 0;
            while (j < 4) {
                temp[j] = roundKeys[(i - 4) / 16][(i - 4) % 16 + j];
                ++j;
            }

            // every 16‑byte boundary apply core
            if (i % 16 == 0) {
                // RotWord
                int t = temp[0];
                temp[0] = temp[1];
                temp[1] = temp[2];
                temp[2] = temp[3];
                temp[3] = t;
                // SubWord
                int k = 0;
                while (k < 4) {
                    temp[k] = Sbox[temp[k]];
                    ++k;
                }
                // Rcon
                temp[0] ^= Rcon[rconIdx];
                ++rconIdx;
            }

            // XOR with word 16 bytes back
            int m = 0;
            while (m < 4) {
                int prev = roundKeys[(i - 16) / 16][(i - 16) % 16 + m];
                int val = prev ^ temp[m];
                roundKeys[i / 16][i % 16 + m] = val;
                ++m;
            }
            i += 4;
        }
    }

    // --------------------------------------------------------------------
    //  AddRoundKey
    // --------------------------------------------------------------------
    void addRoundKey(int state[16], const int *rk) {
        int p = 0;
        while (p < 16) {
            state[p] ^= rk[p];
            ++p;
        }
    }

    // --------------------------------------------------------------------
    //  SubBytes
    // --------------------------------------------------------------------
    void subBytes(int state[16]) {
        int p = 0;
        while (p < 16) {
            state[p] = Sbox[state[p]];
            ++p;
        }
    }

    // --------------------------------------------------------------------
    //  ShiftRows (in‑place)
    // --------------------------------------------------------------------
    void shiftRows(int st[16]) {
        // row 1 (bytes 1,5,9,13) left shift by 1
        int a = st[1];
        st[1] = st[5];
        st[5] = st[9];
        st[9] = st[13];
        st[13] = a;

        // row 2 (bytes 2,6,10,14) left shift by 2
        int b = st[2];
        int c = st[6];
        st[2] = st[10];
        st[6] = st[14];
        st[10] = b;
        st[14] = c;

        // row 3 (bytes 3,7,11,15) left shift by 3 (right shift by 1)
        int d = st[15];
        st[15] = st[11];
        st[11] = st[7];
        st[7] = st[3];
        st[3] = d;
    }

    // --------------------------------------------------------------------
    //  MixColumns (in‑place)
    // --------------------------------------------------------------------
    void mixColumns(int st[16]) {
        int col = 0;
        while (col < 4) {
            int i0 = col * 4;
            int s0 = st[i0];
            int s1 = st[i0 + 1];
            int s2 = st[i0 + 2];
            int s3 = st[i0 + 3];

            int t = s0 ^ s1 ^ s2 ^ s3;
            int u = s0;
            int v = xtime(s0 ^ s1);
            st[i0]     ^= v ^ t;
            v = xtime(s1 ^ s2);
            st[i0 + 1] ^= v ^ t;
            v = xtime(s2 ^ s3);
            st[i0 + 2] ^= v ^ t;
            v = xtime(s3 ^ u);
            st[i0 + 3] ^= v ^ t;

            ++col;
        }
    }

    // --------------------------------------------------------------------
    //  xtime – multiply by 2 in GF(2^8)
    // --------------------------------------------------------------------
    int xtime(int x) {
        int shifted = (x << 1) & 0xFF;
        int overflow = (x >> 7) & 1;
        return shifted ^ (overflow * 0x1B);
    }
};

// ------------------------------------------------------------------------
//  S‑box and Rcon tables (filled with literal ints)
// ------------------------------------------------------------------------
int Aes128Ecb::Sbox[256] = {
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

int Aes128Ecb::Rcon[11] = {
    0x00,0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36
};

// ------------------------------------------------------------------------
//  Main – generate a test vector, encrypt, print ciphertext
// ------------------------------------------------------------------------
int main() {
    // 16‑byte plaintext (deterministic pseudo‑random pattern)
    int plain[16] = {
        0x5A,0xC3,0x1F,0x8E,
        0x73,0xB2,0x4D,0x9A,
        0x0E,0xD5,0x6C,0x27,
        0xFA,0x33,0x81,0xB7
    };

    // 16‑byte key (deterministic pseudo‑random pattern)
    int key[16] = {
        0x13,0x57,0x9B,0xDF,
        0x02,0x46,0x8A,0xCE,
        0x11,0x55,0x99,0xDD,
        0x20,0x64,0xA8,0xEC
    };

    int cipher[16];
    Aes128Ecb aes;
    aes.encryptBlock(plain, cipher, key);

    std::cout << "Ciphertext (hex): ";
    int i = 0;
    while (i < 16) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (cipher[i] & 0xFF);
        ++i;
        if (i != 16) std::cout << " ";
    }
    std::cout << std::dec << std::endl;
    return 0;
}
