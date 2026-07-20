#include <iostream>
#include <iomanip>

using namespace std;

/* LLM input variant 3: zeros-and-ones */

/* --------------- AES-128 OFB (version 3) ----------------- */

/* S‑box (plain int array, no const) */
static int sbox[256] = {
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

/* Round constants (plain int array) */
static int rcon[11] = {0x00,0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36};

/* Helper: multiply by x (0x02) in GF(2^8) */
int xt(int v) {
    int u = v << 1;
    u = (u & 0x100) ? (u ^ 0x11B) : u;
    return u & 0xFF;
}

/* AES class – holds key schedule and provides OFB encryption */
class AesOfb {
    int rk[11][16];                 // round keys (stack‑allocated)

    /* Rotate a word (4‑byte array) left by one byte */
    void rotWord(int w[4]) {
        int t = w[0];
        w[0] = w[1];
        w[1] = w[2];
        w[2] = w[3];
        w[3] = t;
    }

    /* Apply S‑box to a word */
    void subWord(int w[4]) {
        for (int i = 0; i < 4; ++i) w[i] = sbox[w[i]];
    }

    /* Expand 128‑bit key into 11 round keys */
    void expandKey(const int key[16]) {
        for (int i = 0; i < 16; ++i) rk[0][i] = key[i];

        for (int r = 1; r <= 10; ++r) {
            int temp[4];
            for (int i = 0; i < 4; ++i) temp[i] = rk[r-1][12 + i];

            rotWord(temp);
            subWord(temp);
            temp[0] ^= rcon[r];

            for (int i = 0; i < 4; ++i) {
                rk[r][i] = rk[r-1][i] ^ temp[i];
            }
            for (int c = 1; c < 4; ++c) {
                for (int i = 0; i < 4; ++i) {
                    rk[r][c*4 + i] = rk[r-1][c*4 + i] ^ rk[r][(c-1)*4 + i];
                }
            }
        }
    }

    /* AddRoundKey – XOR state with round key */
    void addKey(int st[16], const int *key) {
        for (int i = 0; i < 16; ++i) st[i] ^= key[i];
    }

    /* SubBytes – apply S‑box to each byte */
    void subBytes(int st[16]) {
        for (int i = 0; i < 16; ++i) st[i] = sbox[st[i]];
    }

    /* ShiftRows – cyclic shift each row */
    void shiftRows(int st[16]) {
        int t;

        /* Row 1 */
        t = st[1]; st[1] = st[5]; st[5] = st[9]; st[9] = st[13]; st[13] = t;

        /* Row 2 (swap) */
        t = st[2]; st[2] = st[10]; st[10] = t;
        t = st[6]; st[6] = st[14]; st[14] = t;

        /* Row 3 */
        t = st[3]; st[3] = st[15]; st[15] = st[11]; st[11] = st[7]; st[7] = t;
    }

    /* MixColumns – column‑wise matrix multiplication */
    void mixCols(int st[16]) {
        for (int c = 0; c < 4; ++c) {
            int i0 = c*4, i1 = i0+1, i2 = i0+2, i3 = i0+3;
            int a0 = st[i0], a1 = st[i1], a2 = st[i2], a3 = st[i3];
            int b0 = xt(a0), b1 = xt(a1), b2 = xt(a2), b3 = xt(a3);

            /* Reordered arithmetic, still equivalent */
            st[i0] = b0 ^ (b1 ^ a1) ^ a2 ^ a3;
            st[i1] = a0 ^ b1 ^ (b2 ^ a2) ^ a3;
            st[i2] = a0 ^ a1 ^ b2 ^ (b3 ^ a3);
            st[i3] = (b0 ^ a0) ^ a1 ^ a2 ^ b3;
        }
    }

    /* Encrypt a single 16‑byte block */
    void encryptBlock(const int in[16], int out[16]) {
        int st[16];
        for (int i = 0; i < 16; ++i) st[i] = in[i];

        addKey(st, rk[0]);

        for (int r = 1; r < 10; ++r) {
            subBytes(st);
            shiftRows(st);
            mixCols(st);
            addKey(st, rk[r]);
        }

        subBytes(st);
        shiftRows(st);
        addKey(st, rk[10]);

        for (int i = 0; i < 16; ++i) out[i] = st[i];
    }

public:
    /* Constructor – performs key schedule */
    AesOfb(const int key[16]) {
        expandKey(key);
    }

    /* OFB encryption: generates keystream from IV and XORs with plaintext */
    void ofb(const int iv[16], const int *plain, int *cipher, int len) {
        int cur[16];
        for (int i = 0; i < 16; ++i) cur[i] = iv[i];

        int pos = 0;
        while (pos < len) {
            int ks[16];
            encryptBlock(cur, ks);           // keystream block
            for (int i = 0; i < 16 && pos < len; ++i, ++pos) {
                cipher[pos] = plain[pos] ^ ks[i];
            }
            for (int i = 0; i < 16; ++i) cur[i] = ks[i];   // next IV = keystream
        }
    }
};

/* -------------------- Main (deterministic test) -------------------- */
int main() {
    /* 128‑bit key – alternating zeros and ones */
    int key[16] = {
        0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,
        0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01
    };

    /* Plaintext – 32 bytes, all zeros except occasional ones */
    int plain[32] = {
        0x00,0x01,0x00,0x00, 0x00,0x00,0x01,0x00,
        0x00,0x00,0x00,0x00, 0x01,0x00,0x00,0x00,
        0x00,0x00,0x00,0x01, 0x00,0x00,0x00,0x00,
        0x00,0x01,0x00,0x00, 0x00,0x00,0x00,0x00
    };

    /* IV – all zeros (identity for OFB start) */
    int iv[16] = {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    };

    int cipher[32];
    AesOfb engine(key);
    engine.ofb(iv, plain, cipher, 32);

    cout << "Ciphertext (OFB, AES‑128):" << endl;
    for (int i = 0; i < 32; ++i) {
        if (i % 16 == 0) cout << endl;
        cout << hex << setw(2) << setfill('0') << (cipher[i] & 0xFF) << ' ';
    }
    cout << endl;
    return 0;
}
