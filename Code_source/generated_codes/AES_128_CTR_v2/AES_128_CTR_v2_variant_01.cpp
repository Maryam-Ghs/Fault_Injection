#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <iomanip>

/* LLM input variant 1: minimal-boundary */

class AesCtr128 {
    std::vector< std::vector<int> > roundKey;   // 11 round keys, each 16 bytes
    std::vector<int> sbox;                     // substitution box
    std::vector<int> rcon;                     // round constants

    // Helper: multiply by x in GF(2^8)
    int xtime(int v) {
        int shifted = (v << 1) & 0xff;
        int reduced = ((v >> 7) & 1) * 0x1b;
        return (shifted ^ reduced) & 0xff;
    }

    // Helper: multiplication by 1,2,3 (used in MixColumns)
    int mul(int v, int m) {
        if (m == 1) return v;
        if (m == 2) return xtime(v);
        // m == 3
        return (xtime(v) ^ v) & 0xff;
    }

    // Core AES transformations on a 16‑byte state (in‑place)
    void subBytes(std::vector<int>& st) {
        for (int i = 0; i < 16; ++i)
            st[i] = sbox[st[i]];
    }

    void shiftRows(std::vector<int>& st) {
        int tmp;

        // row 1: shift left by 1
        tmp = st[1];
        st[1] = st[5];
        st[5] = st[9];
        st[9] = st[13];
        st[13] = tmp;

        // row 2: shift left by 2
        tmp = st[2];
        st[2] = st[10];
        st[10] = tmp;
        tmp = st[6];
        st[6] = st[14];
        st[14] = tmp;

        // row 3: shift left by 3 (right by 1)
        tmp = st[15];
        st[15] = st[11];
        st[11] = st[7];
        st[7] = st[3];
        st[3] = tmp;
    }

    void mixColumns(std::vector<int>& st) {
        for (int c = 0; c < 4; ++c) {
            int i0 = c * 4;
            int a0 = st[i0];
            int a1 = st[i0 + 1];
            int a2 = st[i0 + 2];
            int a3 = st[i0 + 3];

            int r0 = mul(a0,2) ^ mul(a1,3) ^ a2 ^ a3;
            int r1 = a0 ^ mul(a1,2) ^ mul(a2,3) ^ a3;
            int r2 = a0 ^ a1 ^ mul(a2,2) ^ mul(a3,3);
            int r3 = mul(a0,3) ^ a1 ^ a2 ^ mul(a3,2);

            st[i0]     = r0 & 0xff;
            st[i0 + 1] = r1 & 0xff;
            st[i0 + 2] = r2 & 0xff;
            st[i0 + 3] = r3 & 0xff;
        }
    }

    void addRoundKey(std::vector<int>& st, const std::vector<int>& rk) {
        for (int i = 0; i < 16; ++i)
            st[i] ^= rk[i];
    }

    // Key schedule for 128‑bit key
    void expandKey(const std::vector<int>& key) {
        roundKey.clear();
        roundKey.push_back(key);               // round 0 key

        for (int round = 1; round <= 10; ++round) {
            std::vector<int> prev = roundKey[round - 1];
            std::vector<int> cur(16);

            // first 4 bytes: rotate, sub, xor Rcon
            int t0 = prev[12];
            int t1 = prev[13];
            int t2 = prev[14];
            int t3 = prev[15];

            // RotWord
            int tmp = t0;
            t0 = t1; t1 = t2; t2 = t3; t3 = tmp;

            // SubWord
            t0 = sbox[t0];
            t1 = sbox[t1];
            t2 = sbox[t2];
            t3 = sbox[t3];

            // Rcon
            t0 ^= rcon[round];

            // generate first word
            cur[0] = prev[0] ^ t0;
            cur[1] = prev[1] ^ t1;
            cur[2] = prev[2] ^ t2;
            cur[3] = prev[3] ^ t3;

            // remaining words
            for (int i = 4; i < 16; ++i)
                cur[i] = prev[i] ^ cur[i - 4];

            roundKey.push_back(cur);
        }
    }

    // Encrypt one block (16 bytes) with the expanded key
    void encryptBlock(std::vector<int>& blk) {
        addRoundKey(blk, roundKey[0]);

        for (int r = 1; r < 10; ++r) {
            subBytes(blk);
            shiftRows(blk);
            mixColumns(blk);
            addRoundKey(blk, roundKey[r]);
        }

        subBytes(blk);
        shiftRows(blk);
        addRoundKey(blk, roundKey[10]);
    }

    // Increment 128‑bit counter (big‑endian)
    void incCounter(std::vector<int>& ctr) {
        for (int i = 15; i >= 0; --i) {
            ctr[i] = (ctr[i] + 1) & 0xff;
            if (ctr[i] != 0) break;
        }
    }

public:
    AesCtr128() {
        // initialise S‑box (non‑const as required)
        sbox = {
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

        // initialise Rcon (non‑const)
        rcon = {0,1,2,4,8,16,32,64,128,27,54};
    }

    // Perform CTR encryption on a vector of plaintext bytes
    std::vector<int> encryptCtr(const std::vector<int>& plain,
                                const std::vector<int>& key,
                                const std::vector<int>& nonce) {
        expandKey(key);

        std::vector<int> output(plain.size());
        std::vector<int> counter = nonce;          // mutable copy

        int blocks = plain.size() / 16;
        for (int b = 0; b < blocks; ++b) {
            std::vector<int> keystream(16);
            for (int i = 0; i < 16; ++i) keystream[i] = counter[i];

            encryptBlock(keystream);

            for (int i = 0; i < 16; ++i) {
                int idx = b * 16 + i;
                output[idx] = (plain[idx] ^ keystream[i]) & 0xff;
            }
            incCounter(counter);
        }
        return output;
    }
};

int main() {
    // deterministic 128‑bit key (minimal example)
    std::vector<int> key = {
        0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
        0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f
    };

    // deterministic nonce (counter start)
    std::vector<int> nonce = {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01
    };

    // minimal plaintext: exactly one block (16 bytes)
    std::vector<int> plaintext = {
        0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,
        0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff
    };

    AesCtr128 engine;
    std::vector<int> ciphertext = engine.encryptCtr(plaintext, key, nonce);

    // print ciphertext as hex
    std::cout << "Ciphertext (" << ciphertext.size() << " bytes):\n";
    for (int i = 0; i < (int)ciphertext.size(); ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (ciphertext[i] & 0xff);
        if ((i + 1) % 16 == 0) std::cout << "\n";
        else std::cout << " ";
    }
    return 0;
}
