#include <iostream>
#include <iomanip>

#define BLK 16
#define RND 10

/* LLM input variant 6: ordered-structured */

/*-------------------------------------------------------------
   AES‑128 in CTR mode – version #9
   Only int and float are used (no unsigned, long, const, double)
--------------------------------------------------------------*/

// -----------------------------------------------------------------
//  S‑box (filled at start, mutable but never changed later)
// -----------------------------------------------------------------
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

// -----------------------------------------------------------------
//  Rcon – round constants (mutable, never altered)
// -----------------------------------------------------------------
int rcon[11] = {0x00,0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36};

// -----------------------------------------------------------------
//  Helper – multiplication in GF(2^8)
// -----------------------------------------------------------------
inline int mul2(int x) {
    int y = (x << 1) & 0xFF;
    y ^= (x & 0x80) ? 0x1B : 0;
    return y;
}
inline int mul3(int x) { return mul2(x) ^ x; }

// -----------------------------------------------------------------
//  Class – AES‑128 core and CTR wrapper
// -----------------------------------------------------------------
class AesCtr {
    int roundKey[44][4];               // 44 words, each 4 bytes
    int sboxTmp[256];                  // local copy of the S‑box
    int rconTmp[11];                   // local copy of Rcon

public:
    AesCtr(const int key[BLK]) {
        // copy tables (no const allowed)
        int i = 0;
        while (i < 256) { sboxTmp[i] = sbox[i]; ++i; }
        i = 0;
        while (i < 11)  { rconTmp[i] = rcon[i]; ++i; }

        keyExpand(key);
    }

    // -----------------------------------------------------------------
    //  Key expansion – produces roundKey[44][4]
    // -----------------------------------------------------------------
    void keyExpand(const int key[BLK]) {
        int i = 0;
        while (i < 4) {
            roundKey[i][0] = key[4*i];
            roundKey[i][1] = key[4*i+1];
            roundKey[i][2] = key[4*i+2];
            roundKey[i][3] = key[4*i+3];
            ++i;
        }

        int r = 1;
        i = 4;
        while (i < 44) {
            int t0 = roundKey[i-1][0];
            int t1 = roundKey[i-1][1];
            int t2 = roundKey[i-1][2];
            int t3 = roundKey[i-1][3];

            if (i % 4 == 0) {
                // RotWord
                int tmp = t0; t0 = t1; t1 = t2; t2 = t3; t3 = tmp;
                // SubWord
                t0 = sboxTmp[t0]; t1 = sboxTmp[t1];
                t2 = sboxTmp[t2]; t3 = sboxTmp[t3];
                // Rcon
                t0 ^= rconTmp[r];
                ++r;
            }

            roundKey[i][0] = roundKey[i-4][0] ^ t0;
            roundKey[i][1] = roundKey[i-4][1] ^ t1;
            roundKey[i][2] = roundKey[i-4][2] ^ t2;
            roundKey[i][3] = roundKey[i-4][3] ^ t3;
            ++i;
        }
    }

    // -----------------------------------------------------------------
    //  Encrypt a single 16‑byte block (in‑place)
    // -----------------------------------------------------------------
    void encryptBlock(int block[BLK]) {
        int state[4][4];                 // column‑major
        int i = 0, c = 0, r = 0;
        while (c < 4) {
            r = 0;
            while (r < 4) {
                state[c][r] = block[i++];
                ++r;
            }
            ++c;
        }

        addRoundKey(state, 0);

        int round = 1;
        while (round < RND) {
            subBytes(state);
            shiftRows(state);
            mixColumns(state);
            addRoundKey(state, round);
            ++round;
        }

        subBytes(state);
        shiftRows(state);
        addRoundKey(state, RND);

        // write back
        i = 0; c = 0;
        while (c < 4) {
            r = 0;
            while (r < 4) {
                block[i++] = state[c][r];
                ++r;
            }
            ++c;
        }
    }

    // -----------------------------------------------------------------
    //  CTR encrypt – plaintext and ciphertext share the same buffer
    // -----------------------------------------------------------------
    void encryptCtr(int *data, int blocks) {
        int counter[BLK];
        // simple 12‑byte nonce of zeros + 4‑byte counter
        int i = 0;
        while (i < BLK) { counter[i] = 0; ++i; }

        int blkIdx = 0;
        while (blkIdx < blocks) {
            int keystream[BLK];
            int j = 0;
            while (j < BLK) { keystream[j] = counter[j]; ++j; }

            encryptBlock(keystream);

            // XOR keystream with plaintext block
            i = 0;
            while (i < BLK) {
                data[blkIdx*BLK + i] ^= keystream[i];
                ++i;
            }

            // increment counter (big‑endian 32‑bit)
            i = BLK - 1;
            while (i >= BLK-4) {
                ++counter[i];
                if (counter[i] <= 0xFF) break;   // no overflow beyond byte
                counter[i] = 0;
                --i;
            }
            ++blkIdx;
        }
    }

private:
    // -----------------------------------------------------------------
    //  Core AES transformations (private)
    // -----------------------------------------------------------------
    void subBytes(int st[4][4]) {
        int c = 0, r = 0;
        while (c < 4) {
            r = 0;
            while (r < 4) {
                st[c][r] = sboxTmp[st[c][r]];
                ++r;
            }
            ++c;
        }
    }

    void shiftRows(int st[4][4]) {
        // row 1
        int tmp = st[0][1];
        st[0][1] = st[1][1];
        st[1][1] = st[2][1];
        st[2][1] = st[3][1];
        st[3][1] = tmp;
        // row 2 (two left shifts)
        tmp = st[0][2];
        st[0][2] = st[2][2];
        st[2][2] = tmp;
        tmp = st[1][2];
        st[1][2] = st[3][2];
        st[3][2] = tmp;
        // row 3 (three left shifts = one right shift)
        tmp = st[3][3];
        st[3][3] = st[2][3];
        st[2][3] = st[1][3];
        st[1][3] = st[0][3];
        st[0][3] = tmp;
    }

    void mixColumns(int st[4][4]) {
        int c = 0;
        while (c < 4) {
            int a0 = st[c][0];
            int a1 = st[c][1];
            int a2 = st[c][2];
            int a3 = st[c][3];
            int r0 = mul2(a0) ^ mul3(a1) ^ a2 ^ a3;
            int r1 = a0 ^ mul2(a1) ^ mul3(a2) ^ a3;
            int r2 = a0 ^ a1 ^ mul2(a2) ^ mul3(a3);
            int r3 = mul3(a0) ^ a1 ^ a2 ^ mul2(a3);
            st[c][0] = r0; st[c][1] = r1; st[c][2] = r2; st[c][3] = r3;
            ++c;
        }
    }

    void addRoundKey(int st[4][4], int round) {
        int c = 0;
        while (c < 4) {
            int k0 = roundKey[round*4 + c][0];
            int k1 = roundKey[round*4 + c][1];
            int k2 = roundKey[round*4 + c][2];
            int k3 = roundKey[round*4 + c][3];
            st[c][0] ^= k0;
            st[c][1] ^= k1;
            st[c][2] ^= k2;
            st[c][3] ^= k3;
            ++c;
        }
    }
};

// -----------------------------------------------------------------
//  Demo – encrypt a tiny message in CTR mode
// -----------------------------------------------------------------
int main() {
    // 128‑bit key: sequential values 0x00–0x0F (ordered)
    int key[BLK] = {
        0x00,0x01,0x02,0x03,
        0x04,0x05,0x06,0x07,
        0x08,0x09,0x0A,0x0B,
        0x0C,0x0D,0x0E,0x0F
    };

    // plaintext – 32 bytes (two blocks), ascending then descending pattern
    int plain[32] = {
        0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
        0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
        0x0F,0x0E,0x0D,0x0C,0x0B,0x0A,0x09,0x08,
        0x07,0x06,0x05,0x04,0x03,0x02,0x01,0x00
    };

    // copy plaintext to ciphertext buffer
    int cipher[32];
    int i = 0;
    while (i < 32) { cipher[i] = plain[i]; ++i; }

    // AES‑CTR object
    AesCtr aes(key);
    // 2 blocks = 32 / 16
    aes.encryptCtr(cipher, 2);

    // output ciphertext as hex
    std::cout << "Ciphertext (hex):\n";
    i = 0;
    while (i < 32) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (cipher[i] & 0xFF) << ' ';
        ++i;
        if (i % BLK == 0) std::cout << '\n';
    }
    return 0;
}
