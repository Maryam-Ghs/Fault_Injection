#include <iostream>
#include <cstdint>

/* LLM input variant 10: large-safe-stress */

class AnubisCipher {
    int roundKey[13][16];
    int Sbox[256];
    int mul2[256];
    int mul3[256];

    void initTables() {
        int i, x, y;
        for (i = 0; i < 256; ++i) {
            x = i;
            if (x == 0) x = 0;
            else {
                int p = 1, q = 1;
                for (int j = 1; j < 255; ++j) {
                    p = ((p << 1) ^ ((p & 0x80) ? 0x11B : 0)) & 0xFF;
                    q ^= p;
                }
                x = q;
            }
            y = x ^ ((x << 1) | (x >> 7));
            y ^= ((x << 2) | (x >> 6));
            y ^= ((x << 3) | (x >> 5));
            y ^= ((x << 4) | (x >> 4));
            y ^= 0x63;
            Sbox[i] = y & 0xFF;
        }
        for (i = 0; i < 256; ++i) {
            int a = i;
            int b = ((a << 1) ^ ((a & 0x80) ? 0x11B : 0)) & 0xFF;
            mul2[i] = b;
            mul3[i] = b ^ a;
        }
    }

    void schedule(const int key[16]) {
        int r, c;
        for (c = 0; c < 16; ++c) roundKey[0][c] = key[c] & 0xFF;
        for (r = 1; r <= 12; ++r) {
            for (c = 0; c < 16; ++c) {
                roundKey[r][c] = roundKey[r-1][c] ^ (r & 0xFF);
            }
        }
    }

    void round(const int in[16], int out[16], const int *rk) {
        int s0 = Sbox[in[0] & 0xFF];
        int s1 = Sbox[in[5] & 0xFF];
        int s2 = Sbox[in[10] & 0xFF];
        int s3 = Sbox[in[15] & 0xFF];

        int s4 = Sbox[in[4] & 0xFF];
        int s5 = Sbox[in[9] & 0xFF];
        int s6 = Sbox[in[14] & 0xFF];
        int s7 = Sbox[in[3] & 0xFF];

        int s8  = Sbox[in[8] & 0xFF];
        int s9  = Sbox[in[13] & 0xFF];
        int s10 = Sbox[in[2] & 0xFF];
        int s11 = Sbox[in[7] & 0xFF];

        int s12 = Sbox[in[12] & 0xFF];
        int s13 = Sbox[in[1] & 0xFF];
        int s14 = Sbox[in[6] & 0xFF];
        int s15 = Sbox[in[11] & 0xFF];

        out[0] = (mul2[s0] ^ mul3[s1] ^ s2 ^ s3) ^ rk[0];
        out[1] = (s0 ^ mul2[s1] ^ mul3[s2] ^ s3) ^ rk[1];
        out[2] = (s0 ^ s1 ^ mul2[s2] ^ mul3[s3]) ^ rk[2];
        out[3] = (mul3[s0] ^ s1 ^ s2 ^ mul2[s3]) ^ rk[3];

        out[4] = (mul2[s4] ^ mul3[s5] ^ s6 ^ s7) ^ rk[4];
        out[5] = (s4 ^ mul2[s5] ^ mul3[s6] ^ s7) ^ rk[5];
        out[6] = (s4 ^ s5 ^ mul2[s6] ^ mul3[s7]) ^ rk[6];
        out[7] = (mul3[s4] ^ s5 ^ s6 ^ mul2[s7]) ^ rk[7];

        out[8]  = (mul2[s8] ^ mul3[s9] ^ s10 ^ s11) ^ rk[8];
        out[9]  = (s8 ^ mul2[s9] ^ mul3[s10] ^ s11) ^ rk[9];
        out[10] = (s8 ^ s9 ^ mul2[s10] ^ mul3[s11]) ^ rk[10];
        out[11] = (mul3[s8] ^ s9 ^ s10 ^ mul2[s11]) ^ rk[11];

        out[12] = (mul2[s12] ^ mul3[s13] ^ s14 ^ s15) ^ rk[12];
        out[13] = (s12 ^ mul2[s13] ^ mul3[s14] ^ s15) ^ rk[13];
        out[14] = (s12 ^ s13 ^ mul2[s14] ^ mul3[s15]) ^ rk[14];
        out[15] = (mul3[s12] ^ s13 ^ s14 ^ mul2[s15]) ^ rk[15];
    }

public:
    AnubisCipher() { initTables(); }

    void encrypt(const int pt[16], int ct[16]) {
        int state[16];
        int temp[16];
        int r;
        for (r = 0; r < 16; ++r) state[r] = (pt[r] ^ roundKey[0][r]) & 0xFF;
        for (r = 1; r <= 11; ++r) {
            round(state, temp, roundKey[r]);
            for (int i = 0; i < 16; ++i) state[i] = temp[i];
        }
        int s0 = Sbox[state[0] & 0xFF];
        int s1 = Sbox[state[5] & 0xFF];
        int s2 = Sbox[state[10] & 0xFF];
        int s3 = Sbox[state[15] & 0xFF];

        int s4 = Sbox[state[4] & 0xFF];
        int s5 = Sbox[state[9] & 0xFF];
        int s6 = Sbox[state[14] & 0xFF];
        int s7 = Sbox[state[3] & 0xFF];

        int s8  = Sbox[state[8] & 0xFF];
        int s9  = Sbox[state[13] & 0xFF];
        int s10 = Sbox[state[2] & 0xFF];
        int s11 = Sbox[state[7] & 0xFF];

        int s12 = Sbox[state[12] & 0xFF];
        int s13 = Sbox[state[1] & 0xFF];
        int s14 = Sbox[state[6] & 0xFF];
        int s15 = Sbox[state[11] & 0xFF];

        ct[0] = (s0 ^ roundKey[12][0]) & 0xFF;
        ct[1] = (s1 ^ roundKey[12][1]) & 0xFF;
        ct[2] = (s2 ^ roundKey[12][2]) & 0xFF;
        ct[3] = (s3 ^ roundKey[12][3]) & 0xFF;

        ct[4] = (s4 ^ roundKey[12][4]) & 0xFF;
        ct[5] = (s5 ^ roundKey[12][5]) & 0xFF;
        ct[6] = (s6 ^ roundKey[12][6]) & 0xFF;
        ct[7] = (s7 ^ roundKey[12][7]) & 0xFF;

        ct[8] = (s8 ^ roundKey[12][8]) & 0xFF;
        ct[9] = (s9 ^ roundKey[12][9]) & 0xFF;
        ct[10] = (s10 ^ roundKey[12][10]) & 0xFF;
        ct[11] = (s11 ^ roundKey[12][11]) & 0xFF;

        ct[12] = (s12 ^ roundKey[12][12]) & 0xFF;
        ct[13] = (s13 ^ roundKey[12][13]) & 0xFF;
        ct[14] = (s14 ^ roundKey[12][14]) & 0xFF;
        ct[15] = (s15 ^ roundKey[12][15]) & 0xFF;
    }

    void setKey(const int key[16]) { schedule(key); }
};

static uint32_t xorshift_state = 2463534242u;
inline uint8_t next_byte() {
    xorshift_state ^= xorshift_state << 13;
    xorshift_state ^= xorshift_state >> 17;
    xorshift_state ^= xorshift_state << 5;
    return static_cast<uint8_t>(xorshift_state & 0xFF);
}

int main() {
    const int ITER = 1000000;   // one million encryptions – safe stress test
    AnubisCipher engine;
    int key[16];
    int plain[16];
    int cipher[16];
    int i;

    // deterministic initial key/plain for first iteration
    for (i = 0; i < 16; ++i) {
        key[i] = i;
        plain[i] = 15 - i;
    }
    engine.setKey(key);
    engine.encrypt(plain, cipher);

    // stress loop: repeatedly generate new deterministic data and encrypt
    for (int t = 1; t < ITER; ++t) {
        for (i = 0; i < 16; ++i) {
            key[i] = next_byte();
            plain[i] = next_byte();
        }
        engine.setKey(key);
        engine.encrypt(plain, cipher);
    }

    std::cout << "Key    : ";
    for (i = 0; i < 16; ++i)
        std::cout << std::hex << ((key[i] & 0xFF) < 16 ? "0" : "") << (key[i] & 0xFF);
    std::cout << std::endl;

    std::cout << "Plain  : ";
    for (i = 0; i < 16; ++i)
        std::cout << std::hex << ((plain[i] & 0xFF) < 16 ? "0" : "") << (plain[i] & 0xFF);
    std::cout << std::endl;

    std::cout << "Cipher : ";
    for (i = 0; i < 16; ++i)
        std::cout << std::hex << ((cipher[i] & 0xFF) < 16 ? "0" : "") << (cipher[i] & 0xFF);
    std::cout << std::endl;

    return 0;
}
