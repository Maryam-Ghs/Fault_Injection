/* LLM input variant 5: duplicate-heavy */
// AES‑128/ECB implementation – version #7
// Restrictions: only int/float, no unsigned, const, double, long.
// Uses heap allocation, manual loop unrolling, reordered arithmetic.

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <ctime>

class AesEcb {
public:
    AesEcb(int *keyBytes) {
        // allocate round keys (11 * 16 bytes)
        rk = new int[176];
        keyExpand(keyBytes);
    }

    ~AesEcb() {
        delete[] rk;
    }

    void encryptBlock(int *plain, int *cipher) {
        // copy plaintext into state (column‑major order)
        int st[16];
        for (int i = 0; i < 16; ++i) st[i] = plain[i];

        // ---------- initial AddRoundKey ----------
        addRoundKey(st, 0);

        // ---------- 9 full rounds (manual unrolling) ----------
        // Round 1
        subBytes(st);
        shiftRows(st);
        mixColumns(st);
        addRoundKey(st, 1);
        // Round 2
        subBytes(st);
        shiftRows(st);
        mixColumns(st);
        addRoundKey(st, 2);
        // Round 3
        subBytes(st);
        shiftRows(st);
        mixColumns(st);
        addRoundKey(st, 3);
        // Round 4
        subBytes(st);
        shiftRows(st);
        mixColumns(st);
        addRoundKey(st, 4);
        // Round 5
        subBytes(st);
        shiftRows(st);
        mixColumns(st);
        addRoundKey(st, 5);
        // Round 6
        subBytes(st);
        shiftRows(st);
        mixColumns(st);
        addRoundKey(st, 6);
        // Round 7
        subBytes(st);
        shiftRows(st);
        mixColumns(st);
        addRoundKey(st, 7);
        // Round 8
        subBytes(st);
        shiftRows(st);
        mixColumns(st);
        addRoundKey(st, 8);
        // Round 9
        subBytes(st);
        shiftRows(st);
        mixColumns(st);
        addRoundKey(st, 9);

        // ---------- final round (no MixColumns) ----------
        subBytes(st);
        shiftRows(st);
        addRoundKey(st, 10);

        // copy state to output
        for (int i = 0; i < 16; ++i) cipher[i] = st[i];
    }

private:
    int *rk; // round keys (176 bytes)

    // ------- S‑box (non‑const) -------
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

    // ------- Rcon (non‑const) -------
    int rcon[10] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36};

    // ------- Helper: multiply by 2 in GF(2^8) -------
    int xtime(int v) {
        int r = v << 1;
        if (v & 0x80) r ^= 0x1b;
        return r & 0xFF;
    }

    // ------- Helper: multiply by 2 or 3 -------
    int mul(int a, int b) {
        // b is only 1,2,3
        if (b == 1) return a;
        if (b == 2) return xtime(a);
        // b == 3
        return xtime(a) ^ a;
    }

    // ------- SubBytes (manual unrolling) -------
    void subBytes(int *st) {
        for (int i = 0; i < 16; ++i) st[i] = sbox[st[i]];
    }

    // ------- ShiftRows (manual) -------
    void shiftRows(int *st) {
        int t;

        // row 1 (shift left 1)
        t = st[1];  st[1]  = st[5];  st[5]  = st[9];  st[9]  = st[13]; st[13] = t;
        // row 2 (shift left 2)
        t = st[2];  st[2]  = st[10]; st[10] = t;
        t = st[6];  st[6]  = st[14]; st[14] = t;
        // row 3 (shift left 3)
        t = st[3];  st[3]  = st[15]; st[15] = st[11]; st[11] = st[7]; st[7] = t;
    }

    // ------- MixColumns (manual unrolling) -------
    void mixColumns(int *st) {
        // column 0
        int a0 = st[0], a1 = st[1], a2 = st[2], a3 = st[3];
        st[0] = mul(a0,2) ^ mul(a1,3) ^ a2 ^ a3;
        st[1] = a0 ^ mul(a1,2) ^ mul(a2,3) ^ a3;
        st[2] = a0 ^ a1 ^ mul(a2,2) ^ mul(a3,3);
        st[3] = mul(a0,3) ^ a1 ^ a2 ^ mul(a3,2);
        // column 1
        a0 = st[4]; a1 = st[5]; a2 = st[6]; a3 = st[7];
        st[4] = mul(a0,2) ^ mul(a1,3) ^ a2 ^ a3;
        st[5] = a0 ^ mul(a1,2) ^ mul(a2,3) ^ a3;
        st[6] = a0 ^ a1 ^ mul(a2,2) ^ mul(a3,3);
        st[7] = mul(a0,3) ^ a1 ^ a2 ^ mul(a3,2);
        // column 2
        a0 = st[8]; a1 = st[9]; a2 = st[10]; a3 = st[11];
        st[8]  = mul(a0,2) ^ mul(a1,3) ^ a2 ^ a3;
        st[9]  = a0 ^ mul(a1,2) ^ mul(a2,3) ^ a3;
        st[10] = a0 ^ a1 ^ mul(a2,2) ^ mul(a3,3);
        st[11] = mul(a0,3) ^ a1 ^ a2 ^ mul(a3,2);
        // column 3
        a0 = st[12]; a1 = st[13]; a2 = st[14]; a3 = st[15];
        st[12] = mul(a0,2) ^ mul(a1,3) ^ a2 ^ a3;
        st[13] = a0 ^ mul(a1,2) ^ mul(a2,3) ^ a3;
        st[14] = a0 ^ a1 ^ mul(a2,2) ^ mul(a3,3);
        st[15] = mul(a0,3) ^ a1 ^ a2 ^ mul(a3,2);
    }

    // ------- AddRoundKey (manual) -------
    void addRoundKey(int *st, int round) {
        int off = round * 16;
        for (int i = 0; i < 16; ++i) st[i] ^= rk[off + i];
    }

    // ------- Key Expansion (manual) -------
    void keyExpand(int *key) {
        // first 16 bytes are the original key
        for (int i = 0; i < 16; ++i) rk[i] = key[i];

        int i = 4; // word index
        while (i < 44) {
            // temp = previous word
            int t0 = rk[(i-1)*4+0];
            int t1 = rk[(i-1)*4+1];
            int t2 = rk[(i-1)*4+2];
            int t3 = rk[(i-1)*4+3];

            // every fourth word, rotate + sub + rcon
            if (i % 4 == 0) {
                // rotate left
                int r = t0;
                t0 = t1; t1 = t2; t2 = t3; t3 = r;
                // sub bytes
                t0 = sbox[t0]; t1 = sbox[t1]; t2 = sbox[t2]; t3 = sbox[t3];
                // rcon
                t0 ^= rcon[(i/4)-1];
            }

            // xor with word i-4
            rk[i*4+0] = rk[(i-4)*4+0] ^ t0;
            rk[i*4+1] = rk[(i-4)*4+1] ^ t1;
            rk[i*4+2] = rk[(i-4)*4+2] ^ t2;
            rk[i*4+3] = rk[(i-4)*4+3] ^ t3;

            ++i;
        }
    }
};

int main() {
    // allocate plaintext and key on heap
    int *plain = new int[16];
    int *key   = new int[16];
    int *cipher = new int[16];

    // deterministic duplicate‑heavy input: many repeated bytes
    // Plaintext: pattern 0xAA,0xAA,0xAA,... (all identical)
    // Key: pattern 0x55,0x55,0x55,... (all identical)
    for (int i = 0; i < 16; ++i) {
        plain[i] = 0xAA; // 170 decimal
        key[i]   = 0x55; //  85 decimal
    }

    // display input
    std::cout << "Plaintext : ";
    for (int i = 0; i < 16; ++i) std::cout << std::hex << std::setw(2) << std::setfill('0') << plain[i];
    std::cout << "\nKey       : ";
    for (int i = 0; i < 16; ++i) std::cout << std::hex << std::setw(2) << std::setfill('0') << key[i];
    std::cout << std::dec << "\n";

    // encrypt
    AesEcb engine(key);
    engine.encryptBlock(plain, cipher);

    // display ciphertext
    std::cout << "Ciphertext: ";
    for (int i = 0; i < 16; ++i) std::cout << std::hex << std::setw(2) << std::setfill('0') << cipher[i];
    std::cout << std::dec << "\n";

    // clean up
    delete[] plain;
    delete[] key;
    delete[] cipher;
    return 0;
}
