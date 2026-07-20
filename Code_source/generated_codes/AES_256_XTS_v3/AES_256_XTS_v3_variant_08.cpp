/* LLM input variant 8: sparse-skewed */
#include <iostream>
#include <iomanip>

class AES_XTS {
public:
    // 256‑bit key = 32 bytes, we store as 32 ints (0‑255)
    int key[32];
    int tweakKey[32];
    // 15 round keys (14 rounds + initial) each 16 bytes
    int rdKey[15][16];

    // ------------------------------------------------------------
    // Constructor: generate keys (sparse, skewed values)
    // ------------------------------------------------------------
    void init_keys() {
        // initialize all to zero
        int i = 0;
        while (i < 32) {
            key[i] = 0;
            tweakKey[i] = 0;
            ++i;
        }
        // scatter a few non‑zero bytes in the data key
        key[5]  = 0x2B;
        key[30] = 0x7E;
        key[31] = 0x15;
        // scatter a few non‑zero bytes in the tweak key
        tweakKey[2]  = 0xAA;
        tweakKey[10] = 0x01;
        tweakKey[31] = 0xFF;
        expand_key();
    }

    // ------------------------------------------------------------
    // Core AES operations (int only)
    // ------------------------------------------------------------
    void sub_bytes(int state[16]) {
        int i = 0;
        while (i < 16) {
            state[i] = sbox[state[i]];
            ++i;
        }
    }

    void shift_rows(int state[16]) {
        int tmp;
        // row 1
        tmp = state[1]; state[1] = state[5]; state[5] = state[9];
        state[9] = state[13]; state[13] = tmp;
        // row 2
        tmp = state[2]; state[2] = state[10]; state[10] = tmp;
        tmp = state[6]; state[6] = state[14]; state[14] = tmp;
        // row 3
        tmp = state[3]; state[3] = state[15]; state[15] = state[11];
        state[11] = state[7]; state[7] = tmp;
    }

    // multiplication in GF(2^8)
    int mul2(int x) { return ((x << 1) ^ ((x & 0x80) ? 0x1B : 0)) & 0xFF; }
    int mul3(int x) { return mul2(x) ^ x; }

    void mix_columns(int state[16]) {
        int i = 0;
        while (i < 4) {
            int a0 = state[4*i], a1 = state[4*i+1], a2 = state[4*i+2], a3 = state[4*i+3];
            int b0 = mul2(a0) ^ mul3(a1) ^ a2 ^ a3;
            int b1 = a0 ^ mul2(a1) ^ mul3(a2) ^ a3;
            int b2 = a0 ^ a1 ^ mul2(a2) ^ mul3(a3);
            int b3 = mul3(a0) ^ a1 ^ a2 ^ mul2(a3);
            state[4*i] = b0; state[4*i+1] = b1; state[4*i+2] = b2; state[4*i+3] = b3;
            ++i;
        }
    }

    void add_round_key(int state[16], int round) {
        int i = 0;
        while (i < 16) {
            state[i] ^= rdKey[round][i];
            ++i;
        }
    }

    // ------------------------------------------------------------
    // Key schedule for AES‑256 (int only)
    // ------------------------------------------------------------
    void expand_key() {
        int i = 0;
        while (i < 8) {               // first 8 words = 32 bytes
            rdKey[0][4*i]   = key[4*i];
            rdKey[0][4*i+1] = key[4*i+1];
            rdKey[0][4*i+2] = key[4*i+2];
            rdKey[0][4*i+3] = key[4*i+3];
            ++i;
        }
        int rcon = 1, round = 1;
        while (round < 15) {
            int temp[4];
            // copy last word
            int j = 0;
            while (j < 4) { temp[j] = rdKey[round-1][12+j]; ++j; }
            // RotWord + SubWord + Rcon
            int t = temp[0];
            temp[0] = sbox[temp[1]] ^ rcon;
            temp[1] = sbox[temp[2]];
            temp[2] = sbox[temp[3]];
            temp[3] = sbox[t];
            // first 4 bytes of new round key
            j = 0;
            while (j < 4) {
                rdKey[round][j] = rdKey[round-1][j] ^ temp[j];
                ++j;
            }
            // next 12 bytes
            j = 4;
            while (j < 16) {
                rdKey[round][j] = rdKey[round-1][j] ^ rdKey[round][j-4];
                ++j;
            }
            // every second round we apply extra SubWord
            if (round % 2 == 0) {
                int k = 0;
                while (k < 4) { temp[k] = sbox[rdKey[round][12+k]]; ++k; }
                k = 0;
                while (k < 4) {
                    rdKey[round][12+k] ^= temp[k];
                    ++k;
                }
            }
            // update rcon
            rcon = mul2(rcon);
            ++round;
        }
    }

    // ------------------------------------------------------------
    // Single block AES encryption (int only)
    // ------------------------------------------------------------
    void encrypt_block(int in[16], int out[16]) {
        int state[16];
        int i = 0;
        while (i < 16) { state[i] = in[i]; ++i; }
        add_round_key(state, 0);
        int round = 1;
        while (round < 14) {
            sub_bytes(state);
            shift_rows(state);
            mix_columns(state);
            add_round_key(state, round);
            ++round;
        }
        sub_bytes(state);
        shift_rows(state);
        add_round_key(state, 14);
        i = 0;
        while (i < 16) { out[i] = state[i]; ++i; }
    }

    // ------------------------------------------------------------
    // XTS encrypt a single 16‑byte unit (sparse inputs)
    // ------------------------------------------------------------
    void encrypt_xts(int plain[16], int cipher[16], int sector) {
        int tweak[16];
        // sector number as little‑endian 128‑bit (here only low 4 bytes used)
        int i = 0;
        while (i < 16) { tweak[i] = 0; ++i; }
        tweak[0] = sector & 0xFF;
        tweak[1] = (sector >> 8) & 0xFF;
        tweak[2] = (sector >> 16) & 0xFF;
        tweak[3] = (sector >> 24) & 0xFF;
        // encrypt tweak with tweakKey
        encrypt_block(tweak, tweak);
        // XOR plaintext with tweak
        i = 0;
        while (i < 16) { tweak[i] ^= plain[i]; ++i; }
        // encrypt the result with data key
        encrypt_block(tweak, cipher);
        // XOR ciphertext with tweak again
        i = 0;
        while (i < 16) { cipher[i] ^= tweak[i]; ++i; }
    }

    // ------------------------------------------------------------
    // Helper: print 16‑byte block as hex
    // ------------------------------------------------------------
    void print_block(const char *label, int blk[16]) {
        std::cout << label;
        int i = 0;
        while (i < 16) {
            std::cout << std::hex << std::setw(2) << std::setfill('0') << (blk[i] & 0xFF);
            ++i;
        }
        std::cout << std::dec << std::endl;
    }

private:
    // S‑box (initialized at runtime to avoid const)
    int sbox[256];
    // ------------------------------------------------------------
    // Fill S‑box once (called from constructor)
    // ------------------------------------------------------------
public:
    void build_sbox() {
        int i = 0;
        while (i < 256) {
            sbox[i] = sbox_raw[i];
            ++i;
        }
    }

private:
    // raw table (static int, not const)
    static int sbox_raw[256];
};

// raw S‑box definition (static storage, no const)
int AES_XTS::sbox_raw[256] = {
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

int main() {
    AES_XTS aes;
    aes.build_sbox();          // populate S‑box
    aes.init_keys();           // generate sparse, skewed keys

    // Test 1: sparse plaintext (two non‑zero bytes), sector 0x00010001
    int pt[16];
    int i = 0;
    while (i < 16) { pt[i] = 0; ++i; }
    pt[0] = 0x01;   // low‑order byte
    pt[15] = 0x02;  // high‑order byte

    int ct[16];
    aes.encrypt_xts(pt, ct, 0x00010001);
    aes.print_block("Ciphertext (sector 0x00010001, sparse PT): ", ct);

    // Test 2: another sparse pattern, sector 0x80000001
    i = 0;
    while (i < 16) { pt[i] = 0; ++i; }
    pt[7] = 0xFF;
    pt[8] = 0xFF;

    aes.encrypt_xts(pt, ct, 0x80000001);
    aes.print_block("Ciphertext (sector 0x80000001, sparse FF PT): ", ct);

    return 0;
}
