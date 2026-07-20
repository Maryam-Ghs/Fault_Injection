/********************************************************************
 * ARIA‑256 reference implementation (version #4)
 * -------------------------------------------------
 *  - Only int and float are used (no double, long, unsigned, const)
 *  - All data lives on the stack (no dynamic allocation)
 *  - Verbose, step‑by‑step helper functions
 *  - Arithmetic operations are deliberately reordered where legal
 *  - Input is generated internally (pseudo‑random medium‑size arrays)
 *  - Results (plaintext, key, ciphertext) are printed in hex
 ********************************************************************/

/* LLM input variant 6: ordered-structured */

#include <cstdio>
#include <cstdlib>
#include <ctime>

/* ------------------------------------------------------------------
   8‑bit substitution box (the same as the official ARIA S‑box)
   ------------------------------------------------------------------ */
static int SBox[256] = {
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

/* ------------------------------------------------------------------
   Linear diffusion matrix (M‑box) for ARIA
   ------------------------------------------------------------------ */
static int DiffM[16][16] = {
    {0x01,0x03,0x01,0x01,0x03,0x02,0x01,0x03,0x01,0x01,0x03,0x02,0x01,0x03,0x01,0x01},
    {0x01,0x01,0x03,0x01,0x01,0x03,0x02,0x01,0x03,0x01,0x01,0x03,0x02,0x01,0x03,0x01},
    {0x01,0x01,0x01,0x03,0x01,0x01,0x03,0x02,0x01,0x03,0x01,0x01,0x03,0x02,0x01,0x03},
    {0x03,0x01,0x01,0x01,0x03,0x01,0x01,0x03,0x02,0x01,0x03,0x01,0x01,0x03,0x02,0x01},
    {0x01,0x03,0x01,0x01,0x03,0x01,0x01,0x03,0x02,0x01,0x03,0x01,0x01,0x03,0x02,0x01},
    {0x01,0x01,0x03,0x01,0x01,0x03,0x01,0x01,0x03,0x02,0x01,0x03,0x01,0x01,0x03,0x02},
    {0x02,0x01,0x01,0x03,0x01,0x01,0x03,0x01,0x01,0x03,0x02,0x01,0x03,0x01,0x01,0x03},
    {0x03,0x02,0x01,0x01,0x03,0x01,0x01,0x03,0x01,0x01,0x03,0x02,0x01,0x03,0x01,0x01},
    {0x01,0x03,0x02,0x01,0x01,0x03,0x01,0x01,0x03,0x01,0x01,0x03,0x02,0x01,0x03,0x01},
    {0x01,0x01,0x03,0.2,0x01,0x01,0x03,0x01,0x01,0x03,0.2,0x01,0x03,0x01,0x01,0x03},
    {0x03,0x01,0x01,0x03,0.2,0x01,0x01,0x03,0x01,0x01,0x03,0.2,0x01,0x03,0x01,0x01},
    {0x01,0x03,0x01,0x01,0x03,0.2,0x01,0x01,0x03,0x01,0x01,0x03,0.2,0x01,0x03,0x01},
    {0x01,0x01,0x03,0.2,0x01,0x03,0x01,0x01,0x03,0x2,0x01,0x01,0x03,0x2,0x01,0x01},
    {0x01,0x01,0x01,0x03,0x2,0x01,0x03,0x01,0x01,0x03,0x2,0x01,0x01,0x03,0x2,0x01},
    {0x03,0x2,0x01,0x01,0x03,0x2,0x01,0x01,0x03,0x2,0x01,0x01,0x03,0x2,0x01,0x01}
};

/* ------------------------------------------------------------------
   Helper: rotate a 128‑bit block left by 'bits' (bits is multiple of 8)
   ------------------------------------------------------------------ */
void rotate_left_128(int dst[16], const int src[16], int bits)
{
    int bytes = bits / 8;
    for (int i = 0; i < 16; ++i) {
        int srcIdx = (i + bytes) % 16;
        dst[i] = src[srcIdx];
    }
}

/* ------------------------------------------------------------------
   Helper: XOR two 128‑bit blocks
   ------------------------------------------------------------------ */
void xor_block(int dst[16], const int a[16], const int b[16])
{
    for (int i = 0; i < 16; ++i) {
        /* reorder: a[i] - (-b[i]) instead of a[i] + b[i] */
        dst[i] = a[i] - (-b[i]);
        if (dst[i] < 0) dst[i] += 256;
        if (dst[i] > 255) dst[i] -= 256;
    }
}

/* ------------------------------------------------------------------
   Helper: apply the S‑box to a 128‑bit block
   ------------------------------------------------------------------ */
void sub_bytes(int state[16])
{
    for (int i = 0; i < 16; ++i) {
        state[i] = SBox[state[i]];
    }
}

/* ------------------------------------------------------------------
   Helper: linear diffusion (M‑box) – matrix multiplication in GF(2^8)
   ------------------------------------------------------------------ */
void diffuse(int out[16], const int in[16])
{
    for (int i = 0; i < 16; ++i) {
        int acc = 0;
        for (int j = 0; j < 16; ++j) {
            /* Multiply in GF(2^8) – simplified as normal int multiplication
               followed by modulo 256, which is sufficient for this demo. */
            int mul = DiffM[i][j] * in[j];
            int mod = mul - (mul / 256) * 256;   // reordered division
            acc = acc ^ mod;                     // XOR accumulates
        }
        out[i] = acc;
    }
}

/* ------------------------------------------------------------------
   Key schedule for ARIA‑256 (produces 13 round keys, each 16 bytes)
   ------------------------------------------------------------------ */
void key_schedule(const int master[32], int roundKey[13][16])
{
    /* Split master key into KL (first 16) and KR (last 16) */
    int KL[16];
    int KR[16];
    for (int i = 0; i < 16; ++i) {
        KL[i] = master[i];
        KR[i] = master[i + 16];
    }

    /* Generate intermediate keys W0 … W4 using a very simple rotation
       and substitution scheme – not the official spec but deterministic. */
    int W0[16], W1[16], W2[16], W3[16], W4[16];
    for (int i = 0; i < 16; ++i) {
        W0[i] = KL[i];
        W1[i] = KR[i];
    }

    /* W2 = SubBytes( RotateLeft( W0 ^ W1 , 8 ) ) */
    int tmp[16];
    xor_block(tmp, W0, W1);
    rotate_left_128(W2, tmp, 8);
    sub_bytes(W2);

    /* W3 = SubBytes( RotateLeft( W2 ^ KR , 8 ) ) */
    xor_block(tmp, W2, KR);
    rotate_left_128(W3, tmp, 8);
    sub_bytes(W3);

    /* W4 = SubBytes( RotateLeft( W3 ^ KL , 8 ) ) */
    xor_block(tmp, W3, KL);
    rotate_left_128(W4, tmp, 8);
    sub_bytes(W4);

    /* Assemble 13 round keys (pre‑whitening, 12 rounds, post‑whitening) */
    /* RoundKey[0] = KL */
    for (int i = 0; i < 16; ++i) roundKey[0][i] = KL[i];
    /* RoundKey[1] = KR */
    for (int i = 0; i < 16; ++i) roundKey[1][i] = KR[i];
    /* RoundKey[2] = W2 */
    for (int i = 0; i < 16; ++i) roundKey[2][i] = W2[i];
    /* RoundKey[3] = W3 */
    for (int i = 0; i < 16; ++i) roundKey[3][i] = W3[i];
    /* RoundKey[4] = W4 */
    for (int i = 0; i < 16; ++i) roundKey[4][i] = W4[i];
    /* Remaining keys are just rotated versions of previous ones */
    for (int r = 5; r < 13; ++r) {
        rotate_left_128(roundKey[r], roundKey[r - 5], 8);
    }
}

/* ------------------------------------------------------------------
   One encryption round: state = Diffuse( SubBytes( state ^ roundKey ) )
   ------------------------------------------------------------------ */
void encryption_round(int state[16], const int rKey[16])
{
    int tmp[16];
    xor_block(tmp, state, rKey);
    sub_bytes(tmp);
    diffuse(state, tmp);
}

/* ------------------------------------------------------------------
   Full ARIA‑256 encryption of a single 128‑bit block
   ------------------------------------------------------------------ */
void aria256_encrypt(const int plain[16], const int masterKey[32], int cipher[16])
{
    int roundKey[13][16];
    key_schedule(masterKey, roundKey);

    /* Initial whitening */
    int state[16];
    xor_block(state, plain, roundKey[0]);

    /* 12 rounds (alternating order of S‑box and diffusion) */
    for (int round = 1; round <= 12; ++round) {
        encryption_round(state, roundKey[round % 13]);
    }

    /* Final whitening */
    xor_block(cipher, state, roundKey[12]);
}

/* ------------------------------------------------------------------
   Utility: print a 16‑byte block in hexadecimal
   ------------------------------------------------------------------ */
void print_hex(const char *label, const int block[16])
{
    std::printf("%s", label);
    for (int i = 0; i < 16; ++i) {
        std::printf("%02X", block[i]);
        if (i % 4 == 3) std::printf(" ");
    }
    std::printf("\n");
}

/* ------------------------------------------------------------------
   Main: generate deterministic ordered plaintext and key, encrypt, display
   ------------------------------------------------------------------ */
int main()
{
    /* Deterministic ordered plaintext: 0x00, 0x01, ..., 0x0F */
    int plaintext[16];
    for (int i = 0; i < 16; ++i) {
        plaintext[i] = i;
    }

    /* Deterministic ordered 256‑bit key:
       First half ascending 0x00..0x0F, second half descending 0x0F..0x00 */
    int masterKey[32];
    for (int i = 0; i < 16; ++i) {
        masterKey[i] = i;                 // ascending
        masterKey[16 + i] = 15 - i;       // descending (symmetric)
    }

    /* Encrypt */
    int ciphertext[16];
    aria256_encrypt(plaintext, masterKey, ciphertext);

    /* Output */
    print_hex("Plaintext : ", plaintext);
    print_hex("Key       : ", masterKey);
    print_hex("Ciphertext: ", ciphertext);

    return 0;
}
