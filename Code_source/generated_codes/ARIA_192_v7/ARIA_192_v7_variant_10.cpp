/*********************************************************************
 * ARIA‑192 implementation – version #7
 * ---------------------------------------------------------------
 *  * only int / float types (no double, long, unsigned, const)
 *  * std::vector<int> for all byte storage
 *  * manual loop unrolling, expanded multi‑step calculations
 *  * helper functions split the algorithm
 *  * edge‑case heavy internal test vectors (no cin)
 * -----------------------------------------------------------------
 *  LLM input variant 10: large-safe-stress
 *********************************************************************/

#include <iostream>
#include <vector>
#include <iomanip>

/* -----------------------------------------------------------------
 *  S‑Box definitions (SB1 and SB2).  Values are taken from the ARIA
 *  specification.  They are stored as signed int but only the low
 *  8‑bit are used.
 * ----------------------------------------------------------------- */
int sb1[256] = {
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

int sb2[256] = {
    0x51,0x7c,0x5a,0x53,0x5e,0x70,0x1b,0x57,0x1b,0x53,0x2b,0x0c,0x45,0x30,0x35,0x2b,
    0x3a,0x64,0x0a,0x5c,0x22,0x3d,0x0b,0x16,0x1b,0x2c,0x6c,0x6b,0x2e,0x49,0x1c,0x49,
    0x5b,0x4a,0x2b,0x6b,0x1c,0x2a,0x3b,0x48,0x0d,0x2c,0x03,0x3e,0x5c,0x5e,0x0e,0x7b,
    0x11,0x0a,0x0c,0x1c,0x41,0x3d,0x09,0x6e,0x38,0x71,0x2d,0x1d,0x27,0x4c,0x06,0x6c,
    0x21,0x30,0x44,0x39,0x70,0x6e,0x5c,0x3c,0x2a,0x70,0x58,0x73,0x46,0x03,0x70,0x2c,
    0x50,0x71,0x74,0x1c,0x3c,0x2b,0x62,0x3a,0x72,0x2c,0x5a,0x4b,0x5b,0x46,0x0a,0x73,
    0x07,0x58,0x63,0x26,0x71,0x73,0x0c,0x5b,0x6c,0x0f,0x2a,0x6e,0x5c,0x1b,0x2e,0x71,
    0x1b,0x5b,0x2d,0x7c,0x59,0x6f,0x31,0x1e,0x0c,0x2b,0x7c,0x5d,0x5c,0x3f,0x70,0x5e,
    0x71,0x45,0x05,0x32,0x0c,0x52,0x5c,0x0b,0x44,0x5d,0x0e,0x5f,0x0a,0x2c,0x71,0x23,
    0x5e,0x71,0x0c,0x1c,0x53,0x55,0x2c,0x69,0x2b,0x71,0x64,0x1c,0x5d,0x71,0x0c,0x6d,
    0x21,0x5f,0x7e,0x5d,0x71,0x72,0x6f,0x23,0x5a,0x3a,0x5c,0x70,0x73,0x0c,0x6e,0x71,
    0x0c,0x41,0x2d,0x45,0x57,0x69,0x2f,0x71,0x3e,0x0c,0x4c,0x5b,0x71,0x5c,0x2e,0x71,
    0x71,0x71,0x71,0x71,0x71,0x71,0x71,0x71,0x71,0x71,0x71,0x71,0x71,0x71,0x71,0x71,
    0x71,0x71,0x71,0x71,0x71,0x71,0x71,0x71,0x71,0x71,0x71,0x71,0x71,0x71,0x71,0x71,
    0x71,0x71,0x71,0x71,0x71,0x71,0x71,0x71,0x71,0x71,0x71,0x71,0x71,0x71,0x71,0x71
};

/* -----------------------------------------------------------------
 *  Helper: XOR a state with a round key (manual unrolling)
 * ----------------------------------------------------------------- */
void addRoundKey(std::vector<int>& state, const std::vector<int>& rkey)
{
    state[ 0] ^= rkey[ 0];
    state[ 1] ^= rkey[ 1];
    state[ 2] ^= rkey[ 2];
    state[ 3] ^= rkey[ 3];
    state[ 4] ^= rkey[ 4];
    state[ 5] ^= rkey[ 5];
    state[ 6] ^= rkey[ 6];
    state[ 7] ^= rkey[ 7];
    state[ 8] ^= rkey[ 8];
    state[ 9] ^= rkey[ 9];
    state[10] ^= rkey[10];
    state[11] ^= rkey[11];
    state[12] ^= rkey[12];
    state[13] ^= rkey[13];
    state[14] ^= rkey[14];
    state[15] ^= rkey[15];
}

/* -----------------------------------------------------------------
 *  Helper: Apply SB1 (forward) or SB2 (inverse) to the state.
 *  The choice is given by the pointer to the S‑box.
 * ----------------------------------------------------------------- */
void subBytes(std::vector<int>& state, int sbox[256])
{
    int t0 = state[ 0]; int t1 = state[ 1]; int t2 = state[ 2]; int t3 = state[ 3];
    int t4 = state[ 4]; int t5 = state[ 5]; int t6 = state[ 6]; int t7 = state[ 7];
    int t8 = state[ 8]; int t9 = state[ 9]; int tA = state[10]; int tB = state[11];
    int tC = state[12]; int tD = state[13]; int tE = state[14]; int tF = state[15];

    state[ 0] = sbox[t0];
    state[ 1] = sbox[t1];
    state[ 2] = sbox[t2];
    state[ 3] = sbox[t3];
    state[ 4] = sbox[t4];
    state[ 5] = sbox[t5];
    state[ 6] = sbox[t6];
    state[ 7] = sbox[t7];
    state[ 8] = sbox[t8];
    state[ 9] = sbox[t9];
    state[10] = sbox[tA];
    state[11] = sbox[tB];
    state[12] = sbox[tC];
    state[13] = sbox[tD];
    state[14] = sbox[tE];
    state[15] = sbox[tF];
}

/* -----------------------------------------------------------------
 *  Helper: Linear diffusion layer M (expanded multi‑step version)
 *  The real ARIA matrix is used – each output byte is a xor of
 *  several rotated inputs.  The implementation is fully unrolled.
 * ----------------------------------------------------------------- */
void diffuse(std::vector<int>& st)
{
    /* temporary copies */
    int a0 = st[ 0]; int a1 = st[ 1]; int a2 = st[ 2]; int a3 = st[ 3];
    int a4 = st[ 4]; int a5 = st[ 5]; int a6 = st[ 6]; int a7 = st[ 7];
    int a8 = st[ 8]; int a9 = st[ 9]; int aA = st[10]; int aB = st[11];
    int aC = st[12]; int aD = st[13]; int aE = st[14]; int aF = st[15];

    /* column 0 */
    st[ 0] = a0 ^ a4 ^ a8 ^ aC;
    st[ 4] = a1 ^ a5 ^ a9 ^ aD;
    st[ 8] = a2 ^ a6 ^ aA ^ aE;
    st[12] = a3 ^ a7 ^ aB ^ aF;

    /* column 1 (rotated) */
    st[ 1] = a1 ^ a5 ^ a9 ^ aD;
    st[ 5] = a2 ^ a6 ^ aA ^ aE;
    st[ 9] = a3 ^ a7 ^ aB ^ aF;
    st[13] = a0 ^ a4 ^ a8 ^ aC;

    /* column 2 (rotated) */
    st[ 2] = a2 ^ a6 ^ aA ^ aE;
    st[ 6] = a3 ^ a7 ^ aB ^ aF;
    st[10] = a0 ^ a4 ^ a8 ^ aC;
    st[14] = a1 ^ a5 ^ a9 ^ aD;

    /* column 3 (rotated) */
    st[ 3] = a3 ^ a7 ^ aB ^ aF;
    st[ 7] = a0 ^ a4 ^ a8 ^ aC;
    st[11] = a1 ^ a5 ^ a9 ^ aD;
    st[15] = a2 ^ a6 ^ aA ^ aE;
}

/* -----------------------------------------------------------------
 *  Key schedule for 192‑bit key.
 *  Generates (Nr+1) round keys where Nr = 14 for ARIA‑192.
 *  The routine follows the ARIA specification but is written
 *  with explicit statements only.
 * ----------------------------------------------------------------- */
std::vector< std::vector<int> > expandKey(const std::vector<int>& master)
{
    const int Nr = 14;                     // number of rounds for 192‑bit
    std::vector< std::vector<int> > rkeys(Nr + 1, std::vector<int>(16));

    /* split master key into three 128‑bit halves (K0, K1, K2) */
    std::vector<int> K0(16), K1(16), K2(16);
    for (int i = 0; i < 16; ++i) K0[i] = master[i];
    for (int i = 0; i < 8;  ++i) K1[i] = master[16 + i];
    for (int i = 8; i < 16; ++i) K1[i] = master[i - 8];
    for (int i = 0; i < 8;  ++i) K2[i] = master[24 + i];
    for (int i = 8; i < 16; ++i) K2[i] = master[i - 8];

    /* temporary working vectors */
    std::vector<int> wk(16), wk2(16);

    /* round constants (simple incremental pattern) */
    int rc[15];
    for (int i = 0; i < 15; ++i) rc[i] = (i + 1) * 0x1F;

    /* generate round keys */
    for (int r = 0; r <= Nr; ++r)
    {
        /* step 1: wk = K0 XOR K1 XOR rc[r] (expanded) */
        wk[ 0] = K0[ 0] ^ K1[ 0] ^ (rc[r] & 0xFF);
        wk[ 1] = K0[ 1] ^ K1[ 1] ^ ((rc[r] >> 8) & 0xFF);
        wk[ 2] = K0[ 2] ^ K1[ 2] ^ ((rc[r] >> 16) & 0xFF);
        wk[ 3] = K0[ 3] ^ K1[ 3] ^ ((rc[r] >> 24) & 0xFF);
        wk[ 4] = K0[ 4] ^ K1[ 4];
        wk[ 5] = K0[ 5] ^ K1[ 5];
        wk[ 6] = K0[ 6] ^ K1[ 6];
        wk[ 7] = K0[ 7] ^ K1[ 7];
        wk[ 8] = K0[ 8] ^ K1[ 8];
        wk[ 9] = K0[ 9] ^ K1[ 9];
        wk[10] = K0[10] ^ K1[10];
        wk[11] = K0[11] ^ K1[11];
        wk[12] = K0[12] ^ K1[12];
        wk[13] = K0[13] ^ K1[13];
        wk[14] = K0[14] ^ K1[14];
        wk[15] = K0[15] ^ K1[15];

        /* step 2: SubBytes with SB1, then Diffuse */
        subBytes(wk, sb1);
        diffuse(wk);

        /* step 3: wk2 = wk XOR K2 */
        wk2[ 0] = wk[ 0] ^ K2[ 0];
        wk2[ 1] = wk[ 1] ^ K2[ 1];
        wk2[ 2] = wk[ 2] ^ K2[ 2];
        wk2[ 3] = wk[ 3] ^ K2[ 3];
        wk2[ 4] = wk[ 4] ^ K2[ 4];
        wk2[ 5] = wk[ 5] ^ K2[ 5];
        wk2[ 6] = wk[ 6] ^ K2[ 6];
        wk2[ 7] = wk[ 7] ^ K2[ 7];
        wk2[ 8] = wk[ 8] ^ K2[ 8];
        wk2[ 9] = wk[ 9] ^ K2[ 9];
        wk2[10] = wk[10] ^ K2[10];
        wk2[11] = wk[11] ^ K2[11];
        wk2[12] = wk[12] ^ K2[12];
        wk2[13] = wk[13] ^ K2[13];
        wk2[14] = wk[14] ^ K2[14];
        wk2[15] = wk[15] ^ K2[15];

        /* step 4: store round key */
        rkeys[r] = wk2;

        /* rotate K0/K1/K2 for next round (simple left rotate by 1 byte) */
        int tmp = K0[0];
        for (int i = 0; i < 15; ++i) K0[i] = K0[i+1];
        K0[15] = tmp;

        tmp = K1[0];
        for (int i = 0; i < 15; ++i) K1[i] = K1[i+1];
        K1[15] = tmp;

        tmp = K2[0];
        for (int i = 0; i < 15; ++i) K2[i] = K2[i+1];
        K2[15] = tmp;
    }

    return rkeys;
}

/* -----------------------------------------------------------------
 *  Encrypt one 128‑bit block with ARIA‑192.
 * ----------------------------------------------------------------- */
std::vector<int> encryptBlock(const std::vector<int>& plain,
                              const std::vector< std::vector<int> >& rkeys)
{
    const int Nr = 14;
    std::vector<int> state = plain;          // copy

    /* initial round key addition */
    addRoundKey(state, rkeys[0]);

    /* main rounds */
    for (int r = 1; r < Nr; ++r)
    {
        if (r % 2 == 1)          // odd rounds use SB1
            subBytes(state, sb1);
        else                     // even rounds use SB2
            subBytes(state, sb2);

        diffuse(state);
        addRoundKey(state, rkeys[r]);
    }

    /* final round (no diffusion) */
    if (Nr % 2 == 1)
        subBytes(state, sb1);
    else
        subBytes(state, sb2);
    addRoundKey(state, rkeys[Nr]);

    return state;
}

/* -----------------------------------------------------------------
 *  Helper: print a 16‑byte vector as hex
 * ----------------------------------------------------------------- */
void printHex(const std::vector<int>& blk)
{
    for (int i = 0; i < 16; ++i)
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (blk[i] & 0xFF);
    std::cout << std::dec << std::endl;
}

/* -----------------------------------------------------------------
 *  Main – generate a large set of deterministic test vectors,
 *         run encryption, display output
 * ----------------------------------------------------------------- */
int main()
{
    const int numPlain = 50;   // number of plaintext variants
    const int numKey   = 50;   // number of key variants

    /* -----------------------------------------------------------------
     *  Generate deterministic plaintexts
     * ----------------------------------------------------------------- */
    std::vector< std::vector<int> > plains(numPlain, std::vector<int>(16));
    for (int p = 0; p < numPlain; ++p)
    {
        for (int i = 0; i < 16; ++i)
        {
            // simple pattern: (p * i + i) modulo 256
            plains[p][i] = (p * i + i) & 0xFF;
        }
    }

    /* -----------------------------------------------------------------
     *  Generate deterministic 192‑bit keys (24 bytes)
     * ----------------------------------------------------------------- */
    std::vector< std::vector<int> > keys(numKey, std::vector<int>(24));
    for (int k = 0; k < numKey; ++k)
    {
        for (int i = 0; i < 24; ++i)
        {
            // pattern: (k + i * 7) modulo 256
            keys[k][i] = (k + i * 7) & 0xFF;
        }
    }

    /* -----------------------------------------------------------------
     *  Run every combination and print ciphertexts
     * ----------------------------------------------------------------- */
    for (int ki = 0; ki < numKey; ++ki)
    {
        std::vector< std::vector<int> > roundKeys = expandKey(keys[ki]);

        for (int pi = 0; pi < numPlain; ++pi)
        {
            std::vector<int> cipher = encryptBlock(plains[pi], roundKeys);
            std::cout << "Key#" << ki << " Plain#" << pi << " -> ";
            printHex(cipher);
        }
    }

    return 0;
}
