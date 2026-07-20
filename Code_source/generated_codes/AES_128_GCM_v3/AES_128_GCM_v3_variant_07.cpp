#include <iostream>
#include <iomanip>

/* LLM input variant 7: reverse-adversarial */

class AES_GCM {
    // ----- S‑Box -------------------------------------------------
    int sbox[256];
    // ----- Rcon --------------------------------------------------
    int rcon[11];
    // ----- Expanded key (44 words = 176 bytes) --------------------
    int roundKey[44];

    // ----- Helper: multiply by 2 in GF(2^8) ----------------------
    int mul2(int x) {
        int r = x << 1;
        if (x & 0x80) r ^= 0x1b;
        return r & 0xff;
    }
    // ----- Helper: multiply by 3 in GF(2^8) ----------------------
    int mul3(int x) { return mul2(x) ^ x; }

    // ----- Initialise static tables (no const allowed) -----------
    void init_tables() {
        // S‑Box (taken from the AES specification)
        int sb[256] = {
            0x63,0x7c,0x77,0x7b,0xf2,0x6b,0x6f,0xc5,0x30,0x01,0x67,0x2b,0xfe,0xd7,0xab,0x76,
            0xca,0x82,0xc9,0x7d,0xfa,0x59,0x47,0xf0,0xad,0xd4,0xa2,0xaf,0x9c,0xa4,0x72,0xc0,
            0xb7,0xfd,0x93,0x26,0x36,0x3f,0xf7,0xcc,0x34,0xa5,0xe5,0xf1,0x71,0xd8,0x31,0x15,
            0x04,0xc7,0x23,0xc3,0x18,0x96,0x05,0x9a,0x07,0x12,0x80,0xe2,0xeb,0x27,0xb2,0x75,
            0x09,0x83,0x2c,0x1a,0x1b,0x6e,0x5a,0xa0,0x52,0x3b,0xd6,0xb3,0x29,0xe3,0x2f,0x84,
            0x53,0xd1,0x00,0xed,0x20,0xfc,0xb1,0x5b,0x6a,0xcb,0xbe,0x39,0x4a,0x4c,0x58,0xcf,
            0xd0,0xef,0xaa,0xfb,0x43,0x4d,0x33,0x85,0x45,0xf9,0x02,0x7f,0x50,0x3c,0x9f,0xa8,
            0x51,0xa3,0x40,0x8f,0x92,0x9d,0x38,0xf5,0xbc,0xb6,0xda,0x21,0x10,0xff,0xf3,0xd2,
            0xcd,0x0c,0x13,0xec,0x5f,0x97,0x44,0x17,0xc4,0xa7,0x7e,0x3d,0x64,0x5d,0x19,0x73,
            0x60,0x81,0x4f,0xdc,0x22,0x2a,0x90,0x88,0x46,0xee,0xb8,0x14,0xde,0x5e,0x0b,0xdb,
            0xe0,0x32,0x3a,0x0a,0x49,0x06,0x24,0x5c,0xc2,0xd3,0xac,0x62,0x91,0x95,0xe4,0x79,
            0xe7,0xc8,0x37,0x6d,0x8d,0xd5,0x4e,0xa9,0x6c,0x56,0xf4,0xea,0x65,0x7a,0xae,0x08,
            0xba,0x78,0x25,0x2e,0x1c,0xa6,0xb4,0xc6,0xe8,0xdd,0x74,0x1f,0x4b,0xbd,0x8b,0x8a,
            0x70,0x3e,0xb5,0x66,0x48,0x03,0xf6,0x0e,0x61,0x35,0x57,0xb9,0x86,0xc1,0x1d,0x9e,
            0xe1,0xf8,0x98,0x11,0x69,0xd9,0x8e,0x94,0x9b,0x1e,0x87,0xe9,0xce,0x55,0x28,0xdf,
            0x8c,0xa1,0x89,0x0d,0xbf,0xe6,0x42,0x68,0x41,0x99,0x2d,0x0f,0xb0,0x54,0xbb,0x16
        };
        // Rcon
        int rc[11] = {0x00,0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1b,0x36};

        for (int i = 0; i < 256; ++i) sbox[i] = sb[i];
        for (int i = 0; i < 11; ++i)  rcon[i] = rc[i];
    }

    // ----- Key expansion (manual unrolling) --------------------
    void key_expansion(const int *key) {
        // first 4 words are the key itself
        roundKey[0] = (key[0] << 24) | (key[1] << 16) | (key[2] << 8) | key[3];
        roundKey[1] = (key[4] << 24) | (key[5] << 16) | (key[6] << 8) | key[7];
        roundKey[2] = (key[8] << 24) | (key[9] << 16) | (key[10] << 8) | key[11];
        roundKey[3] = (key[12] << 24) | (key[13] << 16) | (key[14] << 8) | key[15];

        // generate the remaining 40 words
        for (int i = 4; i < 44; ++i) {
            int temp = roundKey[i - 1];
            if (i % 4 == 0) {
                // RotWord
                temp = ((temp << 8) | ((temp >> 24) & 0xff));
                // SubWord
                temp = (sbox[(temp >> 24) & 0xff] << 24) |
                       (sbox[(temp >> 16) & 0xff] << 16) |
                       (sbox[(temp >> 8) & 0xff] << 8) |
                       (sbox[temp & 0xff]);
                // Rcon
                temp ^= (rcon[i / 4] << 24);
            }
            roundKey[i] = roundKey[i - 4] ^ temp;
        }
    }

    // ----- AddRoundKey -----------------------------------------
    void add_round_key(int *st, int round) {
        st[0] ^= roundKey[round * 4];
        st[1] ^= roundKey[round * 4 + 1];
        st[2] ^= roundKey[round * 4 + 2];
        st[3] ^= roundKey[round * 4 + 3];
    }

    // ----- SubBytes (fully unrolled) ---------------------------
    void sub_bytes(int *st) {
        for (int i = 0; i < 4; ++i) {
            int w = st[i];
            int b0 = sbox[(w >> 24) & 0xff];
            int b1 = sbox[(w >> 16) & 0xff];
            int b2 = sbox[(w >> 8) & 0xff];
            int b3 = sbox[w & 0xff];
            st[i] = (b0 << 24) | (b1 << 16) | (b2 << 8) | b3;
        }
    }

    // ----- ShiftRows (manual byte shuffling) -------------------
    void shift_rows(int *st) {
        int b[16];
        // unpack state (column‑major)
        for (int c = 0; c < 4; ++c) {
            int w = st[c];
            b[4 * c + 0] = (w >> 24) & 0xff;
            b[4 * c + 1] = (w >> 16) & 0xff;
            b[4 * c + 2] = (w >> 8) & 0xff;
            b[4 * c + 3] = w & 0xff;
        }
        // row 0 unchanged
        // row 1 shift left by 1
        int t1 = b[1]; b[1] = b[5]; b[5] = b[9]; b[9] = b[13]; b[13] = t1;
        // row 2 shift left by 2
        int t2 = b[2]; int t6 = b[6];
        b[2] = b[10]; b[6] = b[14]; b[10] = t2; b[14] = t6;
        // row 3 shift left by 3 (right by 1)
        int t3 = b[15];
        b[15] = b[11]; b[11] = b[7]; b[7] = b[3]; b[3] = t3;
        // pack back
        for (int c = 0; c < 4; ++c) {
            st[c] = (b[4 * c + 0] << 24) |
                    (b[4 * c + 1] << 16) |
                    (b[4 * c + 2] << 8) |
                    b[4 * c + 3];
        }
    }

    // ----- MixColumns (fully unrolled) -------------------------
    void mix_columns(int *st) {
        for (int c = 0; c < 4; ++c) {
            int w = st[c];
            int s0 = (w >> 24) & 0xff;
            int s1 = (w >> 16) & 0xff;
            int s2 = (w >> 8) & 0xff;
            int s3 = w & 0xff;
            int m0 = mul2(s0) ^ mul3(s1) ^ s2 ^ s3;
            int m1 = s0 ^ mul2(s1) ^ mul3(s2) ^ s3;
            int m2 = s0 ^ s1 ^ mul2(s2) ^ mul3(s3);
            int m3 = mul3(s0) ^ s1 ^ s2 ^ mul2(s3);
            st[c] = (m0 << 24) | (m1 << 16) | (m2 << 8) | m3;
        }
    }

    // ----- Single‑block AES encryption (10 rounds, unrolled) ----
    void encrypt_block(const int *in, int *out) {
        int st[4];
        // load input (big‑endian)
        for (int i = 0; i < 4; ++i)
            st[i] = (in[4 * i] << 24) | (in[4 * i + 1] << 16) |
                    (in[4 * i + 2] << 8) | in[4 * i + 3];

        add_round_key(st, 0);

        // ---- Rounds 1 … 9 (each: SubBytes, ShiftRows, MixColumns, AddRoundKey) ----
        sub_bytes(st); shift_rows(st); mix_columns(st); add_round_key(st, 1);
        sub_bytes(st); shift_rows(st); mix_columns(st); add_round_key(st, 2);
        sub_bytes(st); shift_rows(st); mix_columns(st); add_round_key(st, 3);
        sub_bytes(st); shift_rows(st); mix_columns(st); add_round_key(st, 4);
        sub_bytes(st); shift_rows(st); mix_columns(st); add_round_key(st, 5);
        sub_bytes(st); shift_rows(st); mix_columns(st); add_round_key(st, 6);
        sub_bytes(st); shift_rows(st); mix_columns(st); add_round_key(st, 7);
        sub_bytes(st); shift_rows(st); mix_columns(st); add_round_key(st, 8);
        sub_bytes(st); shift_rows(st); mix_columns(st); add_round_key(st, 9);

        // ---- Final round (no MixColumns) -------------------------------------------
        sub_bytes(st); shift_rows(st); add_round_key(st, 10);

        // store output (big‑endian)
        for (int i = 0; i < 4; ++i) {
            out[4 * i]     = (st[i] >> 24) & 0xff;
            out[4 * i + 1] = (st[i] >> 16) & 0xff;
            out[4 * i + 2] = (st[i] >> 8) & 0xff;
            out[4 * i + 3] = st[i] & 0xff;
        }
    }

    // ----- GHASH: multiplication in GF(2^128) (bit‑wise) -------
    void ghash(const int *X, const int *H, int *Y) {
        int Z[4] = {0, 0, 0, 0};
        int V[4];
        for (int i = 0; i < 4; ++i) V[i] = H[i];

        // process 128 bits, most‑significant first
        for (int bit = 0; bit < 128; ++bit) {
            int word = bit / 32;
            int shift = 31 - (bit % 32);
            if ((X[word] >> shift) & 1) {
                Z[0] ^= V[0];
                Z[1] ^= V[1];
                Z[2] ^= V[2];
                Z[3] ^= V[3];
            }
            // V = V >> 1 (with reduction)
            int lsb = V[3] & 1;
            V[3] = (V[3] >> 1) | ((V[2] & 1) << 31);
            V[2] = (V[2] >> 1) | ((V[1] & 1) << 31);
            V[1] = (V[1] >> 1) | ((V[0] & 1) << 31);
            V[0] = (V[0] >> 1);
            if (lsb) V[0] ^= 0xe1000000;
        }
        for (int i = 0; i < 4; ++i) Y[i] = Z[i];
    }

    // ----- Increment 32‑bit counter (mod 2^32) -----------------
    void inc32(int *ctr) {
        ctr[3] = (ctr[3] + 1) & 0xffffffff;
    }

public:
    // ----- Demo driver -------------------------------------------
    void run_demo() {
        init_tables();

        // ----- Reversed test vectors (adversarial ordering) -----------------
        int key[16] = {
            0x0f,0x0e,0x0d,0x0c,0x0b,0x0a,0x09,0x08,
            0x07,0x06,0x05,0x04,0x03,0x02,0x01,0x00
        };
        int iv[12] = {
            0x0b,0x0a,0x09,0x08,0x07,0x06,
            0x05,0x04,0x03,0x02,0x01,0x00
        };
        int pt[16] = {
            0x2f,0x2e,0x2d,0x2c,0x2b,0x2a,0x29,0x28,
            0x27,0x26,0x25,0x24,0x23,0x22,0x21,0x20
        };

        // ----- Key schedule ------------------------------------
        key_expansion(key);

        // ----- Compute H = AES(0^128) ---------------------------
        int zero[16] = {0};
        int H[16];
        encrypt_block(zero, H);

        // ----- Build J0 from IV (12‑byte IV) --------------------
        int J0[16];
        for (int i = 0; i < 12; ++i) J0[i] = iv[i];
        J0[12] = 0; J0[13] = 0; J0[14] = 0; J0[15] = 1;

        // ----- Counter block (inc32(J0)) ------------------------
        int ctr[16];
        for (int i = 0; i < 16; ++i) ctr[i] = J0[i];
        inc32(ctr);

        // ----- Encrypt counter -----------------------------------
        int enc_ctr[16];
        encrypt_block(ctr, enc_ctr);

        // ----- Ciphertext = PT XOR enc_ctr -----------------------
        int ct[16];
        for (int i = 0; i < 16; ++i) ct[i] = pt[i] ^ enc_ctr[i];

        // ----- GHASH over ciphertext (single block) -------------
        int X[4];
        for (int i = 0; i < 4; ++i)
            X[i] = (ct[4 * i] << 24) | (ct[4 * i + 1] << 16) |
                   (ct[4 * i + 2] << 8) | ct[4 * i + 3];
        int Hwords[4];
        for (int i = 0; i < 4; ++i)
            Hwords[i] = (H[4 * i] << 24) | (H[4 * i + 1] << 16) |
                        (H[4 * i + 2] << 8) | H[4 * i + 3];
        int GH[4];
        ghash(X, Hwords, GH);

        // ----- S = AES(J0) ---------------------------------------
        int S[16];
        encrypt_block(J0, S);
        int Swords[4];
        for (int i = 0; i < 4; ++i)
            Swords[i] = (S[4 * i] << 24) | (S[4 * i + 1] << 16) |
                        (S[4 * i + 2] << 8) | S[4 * i + 3];

        // ----- Tag = S XOR GH ------------------------------------
        int tag[16];
        for (int i = 0; i < 4; ++i) {
            int w = Swords[i] ^ GH[i];
            tag[4 * i]     = (w >> 24) & 0xff;
            tag[4 * i + 1] = (w >> 16) & 0xff;
            tag[4 * i + 2] = (w >> 8) & 0xff;
            tag[4 * i + 3] = w & 0xff;
        }

        // ----- Output -------------------------------------------------
        std::cout << "Ciphertext: ";
        for (int i = 0; i < 16; ++i)
            std::cout << std::hex << std::setw(2) << std::setfill('0') << ct[i];
        std::cout << std::dec << "\nTag:        ";
        for (int i = 0; i < 16; ++i)
            std::cout << std::hex << std::setw(2) << std::setfill('0') << tag[i];
        std::cout << std::dec << std::endl;
    }
};

int main() {
    AES_GCM demo;
    demo.run_demo();
    return 0;
}
