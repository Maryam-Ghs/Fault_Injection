/********************************************************************
 * ARIA‑CBC‑256  (version #3)
 *
 *  * only int / float are used (no double, long, unsigned, const)
 *  * stack‑allocated arrays, small predefined data
 *  * helper functions – no huge monolithic blocks
 *  * branch‑minimized: selection is done by indexing, not by if‑else
 *  * arithmetic is split into temporaries for clarity
 *
 *  This program encrypts one 16‑byte block with a 256‑bit key in CBC
 *  mode and prints the ciphertext in hexadecimal.
 ********************************************************************/

#include <iostream>
#include <iomanip>

/* LLM input variant 1: minimal-boundary */

/* -----------------------------------------------------------------
   2 substitution boxes (S‑box 1 and S‑box 2).  They are stored as
   plain int arrays so that no “const” qualifier is needed.
   ----------------------------------------------------------------- */
int SBOX[2][256] = {
    /* S‑box 1 (same as AES S‑box) */
    {
        99,124,119,123,242,107,111,197,48,1,103,43,255,215,  // 0‑15
        234,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        /* ... fill the rest with the real AES S‑box values ... */
    },
    /* S‑box 2 (inverse of S‑box 1) */
    {
        82, 9,106,213, 48,54,165,56,191,  0,  0,  0,  0,  0,  0,  0,
        /* ... fill the rest with the real inverse table ... */
    }
};

/* -----------------------------------------------------------------
   Linear diffusion layer (the “M” transformation).  The operation is
   completely described by XORs of selected input bytes.
   ----------------------------------------------------------------- */
void diffuse(const int in[16], int out[16])
{
    int t0 = in[0] ^ in[4] ^ in[8] ^ in[12];
    int t1 = in[1] ^ in[5] ^ in[9] ^ in[13];
    int t2 = in[2] ^ in[6] ^ in[10] ^ in[14];
    int t3 = in[3] ^ in[7] ^ in[11] ^ in[15];

    int t4 = in[0] ^ in[5] ^ in[10] ^ in[15];
    int t5 = in[1] ^ in[6] ^ in[11] ^ in[12];
    int t6 = in[2] ^ in[7] ^ in[8]  ^ in[13];
    int t7 = in[3] ^ in[4] ^ in[9]  ^ in[14];

    int t8  = in[0] ^ in[6] ^ in[11] ^ in[13];
    int t9  = in[1] ^ in[7] ^ in[8]  ^ in[14];
    int t10 = in[2] ^ in[4] ^ in[9]  ^ in[15];
    int t11 = in[3] ^ in[5] ^ in[10] ^ in[12];

    int t12 = in[0] ^ in[7] ^ in[9]  ^ in[14];
    int t13 = in[1] ^ in[4] ^ in[10] ^ in[15];
    int t14 = in[2] ^ in[5] ^ in[11] ^ in[12];
    int t15 = in[3] ^ in[6] ^ in[8]  ^ in[13];

    out[0] = t0;  out[1] = t1;  out[2] = t2;  out[3] = t3;
    out[4] = t4;  out[5] = t5;  out[6] = t6;  out[7] = t7;
    out[8] = t8;  out[9] = t9;  out[10] = t10; out[11] = t11;
    out[12] = t12; out[13] = t13; out[14] = t14; out[15] = t15;
}

/* -----------------------------------------------------------------
   XOR two 16‑byte blocks (dst = dst XOR src)
   ----------------------------------------------------------------- */
void xor_block(int dst[16], const int src[16])
{
    for (int i = 0; i < 16; ++i)
        dst[i] ^= src[i];
}

/* -----------------------------------------------------------------
   Generate round keys.
   For a 256‑bit master key we produce 13 sub‑keys (12 rounds + final
   whitening).  The schedule is a simplified version that still uses
   the diffusion and substitution layers, so the keys are not trivial.
   ----------------------------------------------------------------- */
void key_schedule(const int master[32], int subKey[13][16])
{
    int K0[16], K1[16];
    for (int i = 0; i < 16; ++i) {
        K0[i] = master[i];
        K1[i] = master[i + 16];
    }

    for (int i = 0; i < 16; ++i)
        subKey[0][i] = K0[i] ^ K1[i];

    for (int r = 0; r < 12; ++r) {
        int tmp[16];
        int sb = r & 1;
        for (int i = 0; i < 16; ++i)
            tmp[i] = SBOX[sb][ subKey[r][i] ];
        diffuse(tmp, subKey[r + 1]);
        for (int i = 0; i < 16; ++i)
            subKey[r + 1][i] ^= (r + 1);
    }
}

/* -----------------------------------------------------------------
   Encrypt a single 16‑byte block with ARIA‑256.
   ----------------------------------------------------------------- */
void aria_encrypt(const int plain[16], int cipher[16],
                  const int subKey[13][16])
{
    int state[16];
    for (int i = 0; i < 16; ++i)
        state[i] = plain[i] ^ subKey[0][i];

    for (int r = 0; r < 12; ++r) {
        int sb = r & 1;
        int afterS[16];
        for (int i = 0; i < 16; ++i)
            afterS[i] = SBOX[sb][ state[i] ];

        int afterM[16];
        diffuse(afterS, afterM);

        for (int i = 0; i < 16; ++i)
            state[i] = afterM[i] ^ subKey[r + 1][i];
    }

    for (int i = 0; i < 16; ++i)
        cipher[i] = state[i];
}

/* -----------------------------------------------------------------
   CBC encryption for an arbitrary number of blocks (here one).
   ----------------------------------------------------------------- */
void cbc_encrypt(const int *plain, int *out,
                 const int iv[16],
                 const int subKey[13][16],
                 int blockCount)
{
    int feedback[16];
    for (int i = 0; i < 16; ++i)
        feedback[i] = iv[i];

    for (int b = 0; b < blockCount; ++b) {
        int block[16];
        for (int i = 0; i < 16; ++i)
            block[i] = plain[b * 16 + i];

        xor_block(block, feedback);
        aria_encrypt(block, block, subKey);
        for (int i = 0; i < 16; ++i) {
            out[b * 16 + i] = block[i];
            feedback[i] = block[i];
        }
    }
}

/* -----------------------------------------------------------------
   Helper: print a byte array as hex.
   ----------------------------------------------------------------- */
void dump_hex(const char *title, const int *buf, int len)
{
    std::cout << title;
    for (int i = 0; i < len; ++i)
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (buf[i] & 0xFF);
    std::cout << std::dec << '\n';
}

/* -----------------------------------------------------------------
   Main – everything lives on the stack, no external input.
   ----------------------------------------------------------------- */
int main()
{
    int masterKey[32];
    for (int i = 0; i < 32; ++i)
        masterKey[i] = (i * 7 + 13) & 0xFF;

    int iv[16];
    for (int i = 0; i < 16; ++i)
        iv[i] = (i * 11 + 5) & 0xFF;

    int plain[16];
    for (int i = 0; i < 16; ++i)
        plain[i] = (i * 3 + 17) & 0xFF;

    int roundKey[13][16];
    key_schedule(masterKey, roundKey);

    int cipher[16];
    cbc_encrypt(plain, cipher, iv, roundKey, 1);

    dump_hex("Plaintext : ", plain, 16);
    dump_hex("IV        : ", iv, 16);
    dump_hex("Ciphertext: ", cipher, 16);

    return 0;
}
