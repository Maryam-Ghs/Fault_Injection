#include <cstdio>
#include <cstdlib>

/* LLM input variant 2: small-diverse */

/*-------------------  S‑box & Rcon  -------------------*/
int sbox[256] = {
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

int rcon[11] = {0x00,0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36};

/*-------------------  AES‑128/CBC Class  -------------------*/
class AesCbc {
public:
    int *rk;                     // expanded key (44 * 4 ints)
    int Nb, Nk, Nr;
    AesCbc(int *key) {
        Nb = 4; Nk = 4; Nr = 10;
        rk = new int[4 * Nb * (Nr + 1)];
        keyExp(key);
    }
    ~AesCbc() { delete[] rk; }

    void keyExp(int *k) {
        int i = 0;
        while (i < Nk) {
            rk[4 * i + 0] = k[4 * i + 0];
            rk[4 * i + 1] = k[4 * i + 1];
            rk[4 * i + 2] = k[4 * i + 2];
            rk[4 * i + 3] = k[4 * i + 3];
            i++;
        }
        int t[4];
        while (i < Nb * (Nr + 1)) {
            t[0] = rk[4 * (i - 1) + 0];
            t[1] = rk[4 * (i - 1) + 1];
            t[2] = rk[4 * (i - 1) + 2];
            t[3] = rk[4 * (i - 1) + 3];
            if ((i % Nk) == 0) {
                /* RotWord */
                int tmp = t[0];
                t[0] = t[1]; t[1] = t[2]; t[2] = t[3]; t[3] = tmp;
                /* SubWord */
                t[0] = sbox[t[0]]; t[1] = sbox[t[1]];
                t[2] = sbox[t[2]]; t[3] = sbox[t[3]];
                /* Rcon */
                t[0] = t[0] ^ rcon[i / Nk];
            }
            rk[4 * i + 0] = rk[4 * (i - Nk) + 0] ^ t[0];
            rk[4 * i + 1] = rk[4 * (i - Nk) + 1] ^ t[1];
            rk[4 * i + 2] = rk[4 * (i - Nk) + 2] ^ t[2];
            rk[4 * i + 3] = rk[4 * (i - Nk) + 3] ^ t[3];
            i++;
        }
    }

    void encBlk(int *in, int *out, int *iv) {
        int st[16];
        int p = 0;
        while (p < 16) { st[p] = in[p] ^ iv[p]; p++; }

        /* AddRoundKey (round 0) */
        p = 0;
        while (p < 16) { st[p] = st[p] ^ rk[p]; p++; }

        int r = 1;
        while (r < Nr) {
            /* SubBytes */
            p = 0;
            while (p < 16) { st[p] = sbox[st[p]]; p++; }

            /* ShiftRows (explicit mapping) */
            int tmp[16];
            tmp[0] = st[0];  tmp[4] = st[4];  tmp[8]  = st[8];  tmp[12] = st[12];
            tmp[1] = st[5];  tmp[5] = st[9];  tmp[9]  = st[13]; tmp[13] = st[1];
            tmp[2] = st[10]; tmp[6] = st[14]; tmp[10] = st[2];  tmp[14] = st[6];
            tmp[3] = st[15]; tmp[7] = st[3];  tmp[11] = st[7];  tmp[15] = st[11];
            p = 0; while (p < 16) { st[p] = tmp[p]; p++; }

            /* MixColumns */
            p = 0;
            while (p < 16) {
                int a0 = st[p];
                int a1 = st[p + 1];
                int a2 = st[p + 2];
                int a3 = st[p + 3];

                int b0 = ((a0 << 1) ^ ((a0 >> 7) & 1) * 0x1b) & 0xFF;
                int b1 = ((a1 << 1) ^ ((a1 >> 7) & 1) * 0x1b) & 0xFF;
                int b2 = ((a2 << 1) ^ ((a2 >> 7) & 1) * 0x1b) & 0xFF;
                int b3 = ((a3 << 1) ^ ((a3 >> 7) & 1) * 0x1b) & 0xFF;

                st[p]     = b0 ^ (a1 ^ b1) ^ a2 ^ a3;
                st[p + 1] = a0 ^ b1 ^ (a2 ^ b2) ^ a3;
                st[p + 2] = a0 ^ a1 ^ b2 ^ (a3 ^ b3);
                st[p + 3] = (a0 ^ b0) ^ a1 ^ a2 ^ b3;
                p += 4;
            }

            /* AddRoundKey */
            p = 0;
            while (p < 16) { st[p] = st[p] ^ rk[r * 16 + p]; p++; }
            r++;
        }

        /* Final round (no MixColumns) */
        p = 0; while (p < 16) { st[p] = sbox[st[p]]; p++; }
        int fin[16];
        fin[0] = st[0];  fin[4] = st[4];  fin[8]  = st[8];  fin[12] = st[12];
        fin[1] = st[5];  fin[5] = st[9];  fin[9]  = st[13]; fin[13] = st[1];
        fin[2] = st[10]; fin[6] = st[14]; fin[10] = st[2];  fin[14] = st[6];
        fin[3] = st[15]; fin[7] = st[3];  fin[11] = st[7];  fin[15] = st[11];
        p = 0;
        while (p < 16) { out[p] = fin[p] ^ rk[Nr * 16 + p]; p++; }
    }

    void encrypt(int *pt, int *ct, int blocks, int *iv) {
        int *prev = new int[16];
        int i = 0;
        while (i < 16) { prev[i] = iv[i]; i++; }

        int b = 0;
        while (b < blocks) {
            encBlk(pt + b * 16, ct + b * 16, prev);
            i = 0;
            while (i < 16) { prev[i] = ct[b * 16 + i]; i++; }
            b++;
        }
        delete[] prev;
    }
};

/*-------------------  Main (self‑contained test)  -------------------*/
int main() {
    /* deterministic key: incremental values with a wrap */
    int *key = new int[16];
    for (int i = 0; i < 16; ++i) key[i] = (i * 0x11) & 0xFF;  // 0x00,0x11,0x22,...,0xFF

    /* deterministic IV: descending pattern */
    int *iv = new int[16];
    for (int i = 0; i < 16; ++i) iv[i] = 0xFF - i;  // 0xFF,0xFE,...,0xF0

    /* deterministic plaintext – two blocks with mixed patterns */
    int blocks = 2;
    int *plain = new int[blocks * 16];
    // Block 0: alternating 0xAA and 0x55
    for (int i = 0; i < 16; ++i) plain[i] = (i % 2 == 0) ? 0xAA : 0x55;
    // Block 1: incremental bytes starting from 0x10
    for (int i = 0; i < 16; ++i) plain[16 + i] = 0x10 + i;

    /* encryption */
    AesCbc aes(key);
    int *cipher = new int[blocks * 16];
    aes.encrypt(plain, cipher, blocks, iv);

    /* output */
    printf("Key : ");
    for (int i = 0; i < 16; ++i) printf("%02x", key[i]); printf("\n");
    printf("IV  : ");
    for (int i = 0; i < 16; ++i) printf("%02x", iv[i]); printf("\n");
    printf("Plaintext:\n");
    for (int i = 0; i < blocks * 16; ++i) {
        printf("%02x", plain[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    printf("Ciphertext:\n");
    for (int i = 0; i < blocks * 16; ++i) {
        printf("%02x", cipher[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }

    delete[] key;
    delete[] iv;
    delete[] plain;
    delete[] cipher;
    return 0;
}
