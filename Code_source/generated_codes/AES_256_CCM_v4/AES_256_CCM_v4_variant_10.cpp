#include <iostream>
#include <iomanip>

/* LLM input variant 10: large-safe-stress */

class AES256_CCM {
    int roundKey[15][16];          // 15 round keys (0..14) each 16 bytes
    int sbox[256];                 // substitution box

    // ---- Galois field multiplication helpers (int only) ----
    inline int xtime(int x) { return ((x << 1) ^ ((x >> 7) & 1) * 0x1b) & 0xff; }
    inline int mul2(int x) { return xtime(x); }
    inline int mul3(int x) { return xtime(x) ^ x; }

    // ---- Core AES round operations (fully unrolled) ----
    void subBytes(int *state) {
        state[0] = sbox[state[0]];   state[1] = sbox[state[1]];
        state[2] = sbox[state[2]];   state[3] = sbox[state[3]];
        state[4] = sbox[state[4]];   state[5] = sbox[state[5]];
        state[6] = sbox[state[6]];   state[7] = sbox[state[7]];
        state[8] = sbox[state[8]];   state[9] = sbox[state[9]];
        state[10]= sbox[state[10]];  state[11]= sbox[state[11]];
        state[12]= sbox[state[12]];  state[13]= sbox[state[13]];
        state[14]= sbox[state[14]];  state[15]= sbox[state[15]];
    }

    void shiftRows(int *s) {
        int t;

        // row 1 (rotate left 1)
        t = s[1]; s[1] = s[5]; s[5] = s[9]; s[9] = s[13]; s[13] = t;

        // row 2 (rotate left 2)
        t = s[2]; s[2] = s[10]; s[10] = t;
        t = s[6]; s[6] = s[14]; s[14] = t;

        // row 3 (rotate left 3)
        t = s[3]; s[3] = s[15]; s[15] = s[11]; s[11] = s[7]; s[7] = t;
    }

    void mixColumns(int *s) {
        int a0 = s[0], a1 = s[1], a2 = s[2], a3 = s[3];
        int b0 = mul2(a0) ^ mul3(a1) ^ a2 ^ a3;
        int b1 = a0 ^ mul2(a1) ^ mul3(a2) ^ a3;
        int b2 = a0 ^ a1 ^ mul2(a2) ^ mul3(a3);
        int b3 = mul3(a0) ^ a1 ^ a2 ^ mul2(a3);
        s[0]=b0; s[1]=b1; s[2]=b2; s[3]=b3;

        a0 = s[4]; a1 = s[5]; a2 = s[6]; a3 = s[7];
        b0 = mul2(a0) ^ mul3(a1) ^ a2 ^ a3;
        b1 = a0 ^ mul2(a1) ^ mul3(a2) ^ a3;
        b2 = a0 ^ a1 ^ mul2(a2) ^ mul3(a3);
        b3 = mul3(a0) ^ a1 ^ a2 ^ mul2(a3);
        s[4]=b0; s[5]=b1; s[6]=b2; s[7]=b3;

        a0 = s[8]; a1 = s[9]; a2 = s[10]; a3 = s[11];
        b0 = mul2(a0) ^ mul3(a1) ^ a2 ^ a3;
        b1 = a0 ^ mul2(a1) ^ mul3(a2) ^ a3;
        b2 = a0 ^ a1 ^ mul2(a2) ^ mul3(a3);
        b3 = mul3(a0) ^ a1 ^ a2 ^ mul2(a3);
        s[8]=b0; s[9]=b1; s[10]=b2; s[11]=b3;

        a0 = s[12]; a1 = s[13]; a2 = s[14]; a3 = s[15];
        b0 = mul2(a0) ^ mul3(a1) ^ a2 ^ a3;
        b1 = a0 ^ mul2(a1) ^ mul3(a2) ^ a3;
        b2 = a0 ^ a1 ^ mul2(a2) ^ mul3(a3);
        b3 = mul3(a0) ^ a1 ^ a2 ^ mul2(a3);
        s[12]=b0; s[13]=b1; s[14]=b2; s[15]=b3;
    }

    void addRoundKey(int *state, int round) {
        for (int i = 0; i < 16; ++i) state[i] ^= roundKey[round][i];
    }

    // ---- Key expansion (AES‑256) ----
    void expandKey(const int *key) {
        // copy first 32 bytes (key) into roundKey[0]..roundKey[1]
        for (int i = 0; i < 16; ++i) roundKey[0][i] = key[i];
        for (int i = 0; i < 16; ++i) roundKey[1][i] = key[16 + i];

        int rcon = 0x01;
        for (int i = 2; i < 15; ++i) {
            int tmp[4];
            // take last word of previous round
            for (int j = 0; j < 4; ++j) tmp[j] = roundKey[i-1][12 + j];

            // RotWord + SubWord
            int t = tmp[0];
            tmp[0] = sbox[tmp[1]];
            tmp[1] = sbox[tmp[2]];
            tmp[2] = sbox[tmp[3]];
            tmp[3] = sbox[t];

            // Rcon
            tmp[0] ^= rcon;
            rcon = xtime(rcon);

            // first 4 bytes of round i
            for (int j = 0; j < 4; ++j) roundKey[i][j] = roundKey[i-2][j] ^ tmp[j];

            // remaining 12 bytes
            for (int j = 4; j < 16; ++j) roundKey[i][j] = roundKey[i-2][j] ^ roundKey[i][j-4];
        }
    }

    // ---- Single block encryption (16‑byte) ----
    void encryptBlock(const int *in, int *out) {
        int st[16];
        for (int i = 0; i < 16; ++i) st[i] = in[i] ^ roundKey[0][i]; // initial AddRoundKey

        for (int r = 1; r < 14; ++r) {          // 13 full rounds
            subBytes(st);
            shiftRows(st);
            mixColumns(st);
            addRoundKey(st, r);
        }

        subBytes(st);
        shiftRows(st);
        addRoundKey(st, 14);                    // final round (no MixColumns)

        for (int i = 0; i < 16; ++i) out[i] = st[i];
    }

    // ---- CCM auxiliary helpers ----
    void xorBlock(int *dst, const int *src) {
        for (int i = 0; i < 16; ++i) dst[i] ^= src[i];
    }

    void incCtr(int *ctr) {
        for (int i = 15; i >= 0; --i) {
            ctr[i] = (ctr[i] + 1) & 0xff;
            if (ctr[i]) break;
        }
    }

public:
    AES256_CCM() {
        // initialise S‑box (standard AES values)
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
        for (int i = 0; i < 256; ++i) sbox[i] = sb[i];
    }

    // ---- CCM encrypt (single message) ----
    void ccmEncrypt(
        const int *key,               // 32 bytes
        const int *nonce, int nlen,   // nonce length (7..13)
        const int *adata, int alen,   // associated data
        const int *plain, int plen,   // plaintext
        int *cipher, int *tag, int tlen)   // output ciphertext and tag
    {
        expandKey(key);

        // --------- Build B0 ----------
        int b0[16] = {0};
        b0[0] = ((alen > 0) ? 0x40 : 0x00) | (((tlen-2)/2) << 3) | (15 - nlen - 1);
        for (int i = 0; i < nlen; ++i) b0[1 + i] = nonce[i];
        int q = 15 - nlen;                     // length of length field
        for (int i = 0; i < q; ++i) b0[15 - i] = (plen >> (8*i)) & 0xff;

        // --------- CBC‑MAC ----------
        int mac[16] = {0};
        encryptBlock(b0, mac);                 // mac = AES(K, B0)

        // Associated data length encoding
        if (alen > 0) {
            int aLenEnc[16] = {0};
            aLenEnc[0] = (alen >> 8) & 0xff;
            aLenEnc[1] = alen & 0xff;
            for (int i = 0; i < 2; ++i) mac[i] ^= aLenEnc[i];
            encryptBlock(mac, mac);
            for (int i = 0; i < alen; ++i) {
                mac[i%16] ^= adata[i];
                if ((i%16) == 15) encryptBlock(mac, mac);
            }
            if (alen % 16) encryptBlock(mac, mac);
        }

        // Plaintext MAC
        for (int i = 0; i < plen; ++i) {
            mac[i%16] ^= plain[i];
            if ((i%16) == 15) encryptBlock(mac, mac);
        }
        if (plen % 16) encryptBlock(mac, mac);

        // --------- CTR mode ----------
        int ctr[16] = {0};
        ctr[0] = 15 - nlen - 1;
        for (int i = 0; i < nlen; ++i) ctr[1+i] = nonce[i];
        // counter = 1 for first block
        ctr[15] = 1;

        // encrypt first block to get S0 (used for tag)
        int s0[16];
        encryptBlock(ctr, s0);
        incCtr(ctr);

        // encrypt plaintext
        for (int i = 0; i < plen; ++i) {
            if (i % 16 == 0) encryptBlock(ctr, s0);
            cipher[i] = plain[i] ^ s0[i%16];
            if (i % 16 == 15) incCtr(ctr);
        }

        // compute tag = first tlen bytes of (mac XOR S0)
        for (int i = 0; i < tlen; ++i) tag[i] = (mac[i] ^ s0[i]) & 0xff;
    }
};

int main() {
    // ----- deterministic large test vector (AES‑256‑CCM) -----
    // Key (32 bytes)
    int key[32] = {
        0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,
        0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf,
        0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,
        0xd8,0xd9,0xda,0xdb,0xdc,0xdd,0xde,0xdf
    };
    // Nonce (13 bytes)
    int nonce[13] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c};

    // Associated data (512 bytes)
    const int alen = 512;
    int adata[alen];
    for (int i = 0; i < alen; ++i) adata[i] = i & 0xff;

    // Plaintext (1024 bytes)
    const int plen = 1024;
    int plain[plen];
    for (int i = 0; i < plen; ++i) plain[i] = (0x20 + i) & 0xff;

    const int tlen = 8;          // tag length (8 bytes)

    int cipher[plen];
    int tag[tlen];

    AES256_CCM engine;
    engine.ccmEncrypt(
        key, nonce, 13,
        adata, alen,
        plain, plen,
        cipher, tag, tlen
    );

    // ----- print results -----
    std::cout << "Ciphertext:" << std::hex;
    for (int i = 0; i < plen; ++i) {
        std::cout << std::setfill('0') << std::setw(2) << (cipher[i] & 0xff);
        if ((i+1)%16==0) std::cout << "\n";
        else std::cout << " ";
    }
    std::cout << "\nTag: ";
    for (int i = 0; i < tlen; ++i) {
        std::cout << std::setfill('0') << std::setw(2) << (tag[i] & 0xff);
        if (i+1<tlen) std::cout << " ";
    }
    std::cout << std::dec << std::endl;
    return 0;
}
