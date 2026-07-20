/*******************************************************************
 * AES‑128‑GCM implementation – version #5
 *  * int / float only (no unsigned, double, long, const)
 *  * heap allocation (new / delete)
 *  * class‑based, branch‑minimized style
 *  * edge‑case heavy internal test vectors
 *******************************************************************/

#include <iostream>
#include <iomanip>

/* LLM input variant 9: medium-deterministic-random */

// -----------------------------------------------------------------
//  Static tables (S‑box and multiplication in GF(2^8))
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

int mul2[256];
int mul3[256];

// -----------------------------------------------------------------
//  Helper: fill multiplication tables (branch‑minimized)
// -----------------------------------------------------------------
void fill_mul_tables()
{
    int i = 0;
    while (i < 256) {
        int x = i << 1;
        if (x & 0x100) x ^= 0x11b;
        mul2[i] = x & 0xff;
        mul3[i] = mul2[i] ^ i;
        i = i + 1;
    }
}

// -----------------------------------------------------------------
//  32‑bit rotation left (used in key schedule)
// -----------------------------------------------------------------
int rotl32(int v, int n)
{
    return (v << n) | ((unsigned)v >> (32 - n));
}

// -----------------------------------------------------------------
//  AES‑128‑GCM class
// -----------------------------------------------------------------
class aes_gcm {
public:
    aes_gcm(int* key_bytes, int* iv_bytes) {
        round_key = new int[44];
        hash_sub = new int[4];
        key_expand(key_bytes);
        gen_hash_subkey();
        // build J0 = IV || 0x00000001 (IV assumed 12 bytes)
        int i = 0;
        while (i < 12) { j0[i/4] = (j0[i/4] << 8) | iv_bytes[i]; i = i + 1; }
        j0[3] = 1;
    }
    ~aes_gcm() {
        delete[] round_key;
        delete[] hash_sub;
    }

    // encrypt plaintext (pt_len in bytes, multiple of 16)
    void encrypt(int* pt, int pt_len, int* ct, int* tag) {
        int block_cnt = pt_len >> 4;
        int i = 0;
        while (i < block_cnt) {
            int blk[4];
            int j = 0;
            while (j < 16) { blk[j/4] = (blk[j/4] << 8) | pt[i*16 + j]; j = j + 1; }
            block_enc(blk, blk);
            int k = 0;
            while (k < 16) { ct[i*16 + k] = (blk[k/4] >> (24 - 8*(k%4))) & 0xff; k = k + 1; }
            i = i + 1;
        }
        // GHASH (no AAD, only ciphertext)
        ghash(ct, pt_len, tag);
    }

private:
    int* round_key;   // 44 words (int)
    int* hash_sub;    // H = E(K, 0^128)
    int j0[4] = {0,0,0,0};

    // -----------------------------------------------------------------
    //  Key expansion (AES‑128)
    // -----------------------------------------------------------------
    void key_expand(int* keyb) {
        int i = 0;
        while (i < 4) {
            round_key[i] = (keyb[4*i] << 24) | (keyb[4*i+1] << 16) |
                           (keyb[4*i+2] << 8) | keyb[4*i+3];
            i = i + 1;
        }
        i = 4;
        while (i < 44) {
            int temp = round_key[i-1];
            if ((i % 4) == 0) {
                temp = sub_word(rotl32(temp, 8)) ^ rcon[(i/4)-1];
            }
            round_key[i] = round_key[i-4] ^ temp;
            i = i + 1;
        }
    }

    // -----------------------------------------------------------------
    //  SubWord (used in key schedule)
    // -----------------------------------------------------------------
    int sub_word(int w) {
        int out = 0;
        int i = 0;
        while (i < 4) {
            int b = (w >> (24 - 8*i)) & 0xff;
            out = (out << 8) | sbox[b];
            i = i + 1;
        }
        return out;
    }

    // -----------------------------------------------------------------
    //  Generate hash subkey H = E(K, 0^128)
    // -----------------------------------------------------------------
    void gen_hash_subkey() {
        int zero[4] = {0,0,0,0};
        block_enc(zero, hash_sub);
    }

    // -----------------------------------------------------------------
    //  Single block encryption (AES‑128, 10 rounds)
    // -----------------------------------------------------------------
    void block_enc(int* in, int* out) {
        int state[4];
        int i = 0;
        while (i < 4) { state[i] = in[i] ^ round_key[i]; i = i + 1; }

        i = 1;
        while (i < 10) {
            sub_bytes(state);
            shift_rows(state);
            mix_columns(state);
            int rk = i*4;
            int j = 0;
            while (j < 4) { state[j] ^= round_key[rk + j]; j = j + 1; }
            i = i + 1;
        }
        sub_bytes(state);
        shift_rows(state);
        i = 0;
        while (i < 4) { out[i] = state[i] ^ round_key[40 + i]; i = i + 1; }
    }

    // -----------------------------------------------------------------
    //  SubBytes (using S‑box)
    // -----------------------------------------------------------------
    void sub_bytes(int* st) {
        int i = 0;
        while (i < 4) {
            int v = st[i];
            int out = 0;
            int j = 0;
            while (j < 4) {
                int b = (v >> (24 - 8*j)) & 0xff;
                out = (out << 8) | sbox[b];
                j = j + 1;
            }
            st[i] = out;
            i = i + 1;
        }
    }

    // -----------------------------------------------------------------
    //  ShiftRows (matrix view)
    // -----------------------------------------------------------------
    void shift_rows(int* st) {
        int t0 = (st[0] & 0xff000000) |
                 ((st[1] & 0x00ff0000) >> 8) |
                 ((st[2] & 0x0000ff00) >> 16) |
                 ((st[3] & 0x000000ff) >> 24);
        int t1 = ((st[0] & 0x00ff0000) << 8) |
                 (st[1] & 0x0000ff00) |
                 ((st[2] & 0x000000ff) << 8) |
                 ((st[3] & 0xff000000) >> 24);
        int t2 = ((st[0] & 0x0000ff00) << 16) |
                 ((st[1] & 0x000000ff) << 8) |
                 (st[2] & 0xff000000) |
                 ((st[3] & 0x00ff0000) >> 8);
        int t3 = ((st[0] & 0x000000ff) << 24) |
                 ((st[1] & 0xff000000) >> 8) |
                 ((st[2] & 0x00ff0000) >> 16) |
                 ((st[3] & 0x0000ff00) >> 24);
        st[0] = t0; st[1] = t1; st[2] = t2; st[3] = t3;
    }

    // -----------------------------------------------------------------
    //  MixColumns (using pre‑computed mul2, mul3)
    // -----------------------------------------------------------------
    void mix_columns(int* st) {
        int i = 0;
        while (i < 4) {
            int col = st[i];
            int b0 = (col >> 24) & 0xff;
            int b1 = (col >> 16) & 0xff;
            int b2 = (col >> 8)  & 0xff;
            int b3 = col & 0xff;
            int nb0 = mul2[b0] ^ mul3[b1] ^ b2 ^ b3;
            int nb1 = b0 ^ mul2[b1] ^ mul3[b2] ^ b3;
            int nb2 = b0 ^ b1 ^ mul2[b2] ^ mul3[b3];
            int nb3 = mul3[b0] ^ b1 ^ b2 ^ mul2[b3];
            st[i] = (nb0 << 24) | (nb1 << 16) | (nb2 << 8) | nb3;
            i = i + 1;
        }
    }

    // -----------------------------------------------------------------
    //  GHASH (no AAD, only ciphertext)
    // -----------------------------------------------------------------
    void ghash(int* ct, int ct_len, int* tag) {
        int y[4] = {0,0,0,0};
        int blk_cnt = ct_len >> 4;
        int i = 0;
        while (i < blk_cnt) {
            int blk[4] = {0,0,0,0};
            int j = 0;
            while (j < 16) {
                blk[j/4] = (blk[j/4] << 8) | ct[i*16 + j];
                j = j + 1;
            }
            xor128(y, blk);
            mul128(y, hash_sub);
            i = i + 1;
        }
        // length block (bits)
        int lenblk[4] = {0,0,0, (ct_len<<3) & 0xffffffff};
        xor128(y, lenblk);
        mul128(y, hash_sub);
        // compute final tag = E(K, J0) xor y
        int ek[4];
        block_enc(j0, ek);
        xor128(ek, y);
        int k = 0;
        while (k < 4) { tag[k] = ek[k]; k = k + 1; }
    }

    // -----------------------------------------------------------------
    //  XOR two 128‑bit values (int[4])
    // -----------------------------------------------------------------
    void xor128(int* a, int* b) {
        int i = 0;
        while (i < 4) { a[i] ^= b[i]; i = i + 1; }
    }

    // -----------------------------------------------------------------
    //  Multiply two elements in GF(2^128) (shift‑and‑xor)
    // -----------------------------------------------------------------
    void mul128(int* X, int* Y) {
        int Z[4] = {0,0,0,0};
        int V[4];
        int i = 0;
        while (i < 4) { V[i] = X[i]; i = i + 1; }
        int bit = 0;
        while (bit < 128) {
            int idx = bit >> 5;
            int off = 31 - (bit & 31);
            int xb = (Y[idx] >> off) & 1;
            int mask = -xb;
            int j = 0;
            while (j < 4) {
                Z[j] ^= V[j] & mask;
                j = j + 1;
            }
            int carry = (V[0] >> 31) & 1;
            int k = 0;
            while (k < 3) {
                V[k] = (V[k] << 1) | ((V[k+1] >> 31) & 1);
                k = k + 1;
            }
            V[3] = V[3] << 1;
            if (carry) {
                V[0] ^= 0xe1 << 24;
            }
            bit = bit + 1;
        }
        int i2 = 0;
        while (i2 < 4) { X[i2] = Z[i2]; i2 = i2 + 1; }
    }

    // -----------------------------------------------------------------
    //  Rcon table (no const)
    // -----------------------------------------------------------------
    int rcon[10] = {0x01000000,0x02000000,0x04000000,0x08000000,
                    0x10000000,0x20000000,0x40000000,0x80000000,
                    0x1b000000,0x36000000};
};

// -----------------------------------------------------------------
//  Helper: print byte array as hex
// -----------------------------------------------------------------
void print_hex(const char* label, int* data, int len)
{
    std::cout << label;
    int i = 0;
    while (i < len) {
        int byte = data[i];
        std::cout << std::hex << std::setw(2) << std::setfill('0') << byte;
        i = i + 1;
    }
    std::cout << std::dec << std::endl;
}

// -----------------------------------------------------------------
//  Main – medium‑deterministic‑random test vectors
// -----------------------------------------------------------------
int main()
{
    fill_mul_tables();

    // ---- Test vector 1: zero‑length plaintext with pseudo‑random key/iv ----
    int key1[16];
    int iv1[12];
    int i = 0;
    while (i < 16) { key1[i] = ((i * 31) + 7) & 0xff; i = i + 1; }
    i = 0;
    while (i < 12) { iv1[i] = ((i * 17) + 13) & 0xff; i = i + 1; }

    aes_gcm ctx1(key1, iv1);
    int tag1[16] = {0};
    ctx1.encrypt(nullptr, 0, nullptr, tag1);
    print_hex("Tag (zero‑len PT, rand key/iv): ", tag1, 16);

    // ---- Test vector 2: full‑block of pseudo‑random data ----
    int key2[16];
    int iv2[12];
    i = 0;
    while (i < 16) { key2[i] = ((i * 0x5a) + 0x33) & 0xff; i = i + 1; }
    i = 0;
    while (i < 12) { iv2[i] = ((i * 0x23) + 0x77) & 0xff; i = i + 1; }

    aes_gcm ctx2(key2, iv2);
    int pt2[16];
    i = 0;
    while (i < 16) { pt2[i] = ((i * 123) + 45) & 0xff; i = i + 1; }
    int ct2[16];
    int tag2[16] = {0};
    ctx2.encrypt(pt2, 16, ct2, tag2);
    print_hex("Ciphertext (rand block): ", ct2, 16);
    print_hex("Tag (rand block): ", tag2, 16);

    // ---- Test vector 3: 80‑byte incremental pattern ----
    int key3[16];
    int iv3[12];
    i = 0;
    while (i < 16) { key3[i] = (i * i) & 0xff; i = i + 1; }
    i = 0;
    while (i < 12) { iv3[i] = (0x20 + i * 3) & 0xff; i = i + 1; }

    aes_gcm ctx3(key3, iv3);
    int pt3[80];
    i = 0;
    while (i < 80) { pt3[i] = ((i * 7) + 11) & 0xff; i = i + 1; }
    int ct3[80];
    int tag3[16] = {0};
    ctx3.encrypt(pt3, 80, ct3, tag3);
    print_hex("Ciphertext (80‑byte pattern): ", ct3, 80);
    print_hex("Tag (80‑byte pattern): ", tag3, 16);

    return 0;
}
