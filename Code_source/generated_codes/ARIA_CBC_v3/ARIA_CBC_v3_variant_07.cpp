#include <cstdio>

/* LLM input variant 7: reverse-adversarial */

int main()
{
    /* -------------------------------------------------------------
       Simple ARIA‑CBC implementation (version #3)
       - only int / float types
       - all data on the stack
       - manual loop unrolling and reordered arithmetic
       - edge‑case heavy internal test vectors
       ------------------------------------------------------------- */

    /* 1.  S‑box (full 256‑entry table) */
    int S[256] = {
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

    /* 2.  Fixed 128‑bit key (16 bytes) – reversed order for adversarial effect */
    int K[16] = {
        0xff,0xee,0xdd,0xcc,0xbb,0xaa,0x99,0x88,
        0x77,0x66,0x55,0x44,0x33,0x22,0x11,0x00
    };

    /* 3.  Simple round‑key schedule (12 rounds + pre‑whitening) */
    int RK[13][16];
    int RC[13] = {0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc};

    /*   Manual unrolled key expansion */
    /*   RK[0] = K */
    RK[0][0] = K[0];  RK[0][1] = K[1];  RK[0][2] = K[2];  RK[0][3] = K[3];
    RK[0][4] = K[4];  RK[0][5] = K[5];  RK[0][6] = K[6];  RK[0][7] = K[7];
    RK[0][8] = K[8];  RK[0][9] = K[9];  RK[0][10]= K[10]; RK[0][11]= K[11];
    RK[0][12]= K[12]; RK[0][13]= K[13]; RK[0][14]= K[14]; RK[0][15]= K[15];

    /*   RK[i] = K xor RC[i] (applied to every byte) */
    int i;
    for (i = 1; i < 13; ++i) {
        int r = RC[i];
        RK[i][0] = K[0] ^ r;  RK[i][1] = K[1] ^ r;  RK[i][2] = K[2] ^ r;  RK[i][3] = K[3] ^ r;
        RK[i][4] = K[4] ^ r;  RK[i][5] = K[5] ^ r;  RK[i][6] = K[6] ^ r;  RK[i][7] = K[7] ^ r;
        RK[i][8] = K[8] ^ r;  RK[i][9] = K[9] ^ r;  RK[i][10]= K[10]^ r;  RK[i][11]= K[11]^ r;
        RK[i][12]= K[12]^ r;  RK[i][13]= K[13]^ r;  RK[i][14]= K[14]^ r;  RK[i][15]= K[15]^ r;
    }

    /* 4.  Edge‑case test vectors (plain‑text blocks) – reversed order */
    /*    - case A : zero length (nothing to encrypt) */
    /*    - case B : exactly one block (now descending values) */
    /*    - case C : two blocks, first descending, second ascending */
    int blocks[2][16];
    /* block B (now descending) */
    blocks[0][0] = 0xff; blocks[0][1] = 0xee; blocks[0][2] = 0xdd; blocks[0][3] = 0xcc;
    blocks[0][4] = 0xbb; blocks[0][5] = 0xaa; blocks[0][6] = 0x99; blocks[0][7] = 0x88;
    blocks[0][8] = 0x77; blocks[0][9] = 0x66; blocks[0][10]= 0x55; blocks[0][11]= 0x44;
    blocks[0][12]= 0x33; blocks[0][13]= 0x22; blocks[0][14]= 0x11; blocks[0][15]= 0x00;
    /* block C (ascending) */
    blocks[1][0] = 0x00; blocks[1][1] = 0x11; blocks[1][2] = 0x22; blocks[1][3] = 0x33;
    blocks[1][4] = 0x44; blocks[1][5] = 0x55; blocks[1][6] = 0x66; blocks[1][7] = 0x77;
    blocks[1][8] = 0x88; blocks[1][9] = 0x99; blocks[1][10]= 0xaa; blocks[1][11]= 0xbb;
    blocks[1][12]= 0xcc; blocks[1][13]= 0xdd; blocks[1][14]= 0xee; blocks[1][15]= 0xff;

    /* 5.  Initialization vector (IV) – reversed order */
    int IV[16] = {
        0x21,0x43,0x65,0x87,0xa9,0xcb,0xed,0x0f,
        0xf0,0xde,0xbc,0x9a,0x78,0x56,0x34,0x12
    };

    /* 6.  CBC encryption */
    int prev[16];
    /* copy IV to prev (manual) */
    prev[0] = IV[0];  prev[1] = IV[1];  prev[2] = IV[2];  prev[3] = IV[3];
    prev[4] = IV[4];  prev[5] = IV[5];  prev[6] = IV[6];  prev[7] = IV[7];
    prev[8] = IV[8];  prev[9] = IV[9];  prev[10]= IV[10]; prev[11]= IV[11];
    prev[12]= IV[12]; prev[13]= IV[13]; prev[14]= IV[14]; prev[15]= IV[15];

    /* 7.  Process each test case */
    /* case A : length 0 -> nothing printed */
    printf("Case A (empty plaintext): <no output>\n\n");

    /* case B : single block */
    printf("Case B (1 block):\n");
    {
        int state[16];
        /* XOR with IV (CBC) – reordered: xor first, then substitute */
        state[0] = blocks[0][0] ^ prev[0];
        state[1] = blocks[0][1] ^ prev[1];
        state[2] = blocks[0][2] ^ prev[2];
        state[3] = blocks[0][3] ^ prev[3];
        state[4] = blocks[0][4] ^ prev[4];
        state[5] = blocks[0][5] ^ prev[5];
        state[6] = blocks[0][6] ^ prev[6];
        state[7] = blocks[0][7] ^ prev[7];
        state[8] = blocks[0][8] ^ prev[8];
        state[9] = blocks[0][9] ^ prev[9];
        state[10]= blocks[0][10]^ prev[10];
        state[11]= blocks[0][11]^ prev[11];
        state[12]= blocks[0][12]^ prev[12];
        state[13]= blocks[0][13]^ prev[13];
        state[14]= blocks[0][14]^ prev[14];
        state[15]= blocks[0][15]^ prev[15];

        /* 12 encryption rounds – fully unrolled */
        /* Round 0 */
        state[0] = S[state[0]] ^ RK[0][0];
        state[1] = S[state[1]] ^ RK[0][1];
        state[2] = S[state[2]] ^ RK[0][2];
        state[3] = S[state[3]] ^ RK[0][3];
        state[4] = S[state[4]] ^ RK[0][4];
        state[5] = S[state[5]] ^ RK[0][5];
        state[6] = S[state[6]] ^ RK[0][6];
        state[7] = S[state[7]] ^ RK[0][7];
        state[8] = S[state[8]] ^ RK[0][8];
        state[9] = S[state[9]] ^ RK[0][9];
        state[10]= S[state[10]]^ RK[0][10];
        state[11]= S[state[11]]^ RK[0][11];
        state[12]= S[state[12]]^ RK[0][12];
        state[13]= S[state[13]]^ RK[0][13];
        state[14]= S[state[14]]^ RK[0][14];
        state[15]= S[state[15]]^ RK[0][15];
        /* Round 1 */
        state[0] = S[state[0]] ^ RK[1][0];
        state[1] = S[state[1]] ^ RK[1][1];
        state[2] = S[state[2]] ^ RK[1][2];
        state[3] = S[state[3]] ^ RK[1][3];
        state[4] = S[state[4]] ^ RK[1][4];
        state[5] = S[state[5]] ^ RK[1][5];
        state[6] = S[state[6]] ^ RK[1][6];
        state[7] = S[state[7]] ^ RK[1][7];
        state[8] = S[state[8]] ^ RK[1][8];
        state[9] = S[state[9]] ^ RK[1][9];
        state[10]= S[state[10]]^ RK[1][10];
        state[11]= S[state[11]]^ RK[1][11];
        state[12]= S[state[12]]^ RK[1][12];
        state[13]= S[state[13]]^ RK[1][13];
        state[14]= S[state[14]]^ RK[1][14];
        state[15]= S[state[15]]^ RK[1][15];
        /* Round 2 */
        state[0] = S[state[0]] ^ RK[2][0];
        state[1] = S[state[1]] ^ RK[2][1];
        state[2] = S[state[2]] ^ RK[2][2];
        state[3] = S[state[3]] ^ RK[2][3];
        state[4] = S[state[4]] ^ RK[2][4];
        state[5] = S[state[5]] ^ RK[2][5];
        state[6] = S[state[6]] ^ RK[2][6];
        state[7] = S[state[7]] ^ RK[2][7];
        state[8] = S[state[8]] ^ RK[2][8];
        state[9] = S[state[9]] ^ RK[2][9];
        state[10]= S[state[10]]^ RK[2][10];
        state[11]= S[state[11]]^ RK[2][11];
        state[12]= S[state[12]]^ RK[2][12];
        state[13]= S[state[13]]^ RK[2][13];
        state[14]= S[state[14]]^ RK[2][14];
        state[15]= S[state[15]]^ RK[2][15];
        /* Round 3 */
        state[0] = S[state[0]] ^ RK[3][0];
        state[1] = S[state[1]] ^ RK[3][1];
        state[2] = S[state[2]] ^ RK[3][2];
        state[3] = S[state[3]] ^ RK[3][3];
        state[4] = S[state[4]] ^ RK[3][4];
        state[5] = S[state[5]] ^ RK[3][5];
        state[6] = S[state[6]] ^ RK[3][6];
        state[7] = S[state[7]] ^ RK[3][7];
        state[8] = S[state[8]] ^ RK[3][8];
        state[9] = S[state[9]] ^ RK[3][9];
        state[10]= S[state[10]]^ RK[3][10];
        state[11]= S[state[11]]^ RK[3][11];
        state[12]= S[state[12]]^ RK[3][12];
        state[13]= S[state[13]]^ RK[3][13];
        state[14]= S[state[14]]^ RK[3][14];
        state[15]= S[state[15]]^ RK[3][15];
        /* Round 4 */
        state[0] = S[state[0]] ^ RK[4][0];
        state[1] = S[state[1]] ^ RK[4][1];
        state[2] = S[state[2]] ^ RK[4][2];
        state[3] = S[state[3]] ^ RK[4][3];
        state[4] = S[state[4]] ^ RK[4][4];
        state[5] = S[state[5]] ^ RK[4][5];
        state[6] = S[state[6]] ^ RK[4][6];
        state[7] = S[state[7]] ^ RK[4][7];
        state[8] = S[state[8]] ^ RK[4][8];
        state[9] = S[state[9]] ^ RK[4][9];
        state[10]= S[state[10]]^ RK[4][10];
        state[11]= S[state[11]]^ RK[4][11];
        state[12]= S[state[12]]^ RK[4][12];
        state[13]= S[state[13]]^ RK[4][13];
        state[14]= S[state[14]]^ RK[4][14];
        state[15]= S[state[15]]^ RK[4][15];
        /* Round 5 */
        state[0] = S[state[0]] ^ RK[5][0];
        state[1] = S[state[1]] ^ RK[5][1];
        state[2] = S[state[2]] ^ RK[5][2];
        state[3] = S[state[3]] ^ RK[5][3];
        state[4] = S[state[4]] ^ RK[5][4];
        state[5] = S[state[5]] ^ RK[5][5];
        state[6] = S[state[6]] ^ RK[5][6];
        state[7] = S[state[7]] ^ RK[5][7];
        state[8] = S[state[8]] ^ RK[5][8];
        state[9] = S[state[9]] ^ RK[5][9];
        state[10]= S[state[10]]^ RK[5][10];
        state[11]= S[state[11]]^ RK[5][11];
        state[12]= S[state[12]]^ RK[5][12];
        state[13]= S[state[13]]^ RK[5][13];
        state[14]= S[state[14]]^ RK[5][14];
        state[15]= S[state[15]]^ RK[5][15];
        /* Round 6 */
        state[0] = S[state[0]] ^ RK[6][0];
        state[1] = S[state[1]] ^ RK[6][1];
        state[2] = S[state[2]] ^ RK[6][2];
        state[3] = S[state[3]] ^ RK[6][3];
        state[4] = S[state[4]] ^ RK[6][4];
        state[5] = S[state[5]] ^ RK[6][5];
        state[6] = S[state[6]] ^ RK[6][6];
        state[7] = S[state[7]] ^ RK[6][7];
        state[8] = S[state[8]] ^ RK[6][8];
        state[9] = S[state[9]] ^ RK[6][9];
        state[10]= S[state[10]]^ RK[6][10];
        state[11]= S[state[11]]^ RK[6][11];
        state[12]= S[state[12]]^ RK[6][12];
        state[13]= S[state[13]]^ RK[6][13];
        state[14]= S[state[14]]^ RK[6][14];
        state[15]= S[state[15]]^ RK[6][15];
        /* Round 7 */
        state[0] = S[state[0]] ^ RK[7][0];
        state[1] = S[state[1]] ^ RK[7][1];
        state[2] = S[state[2]] ^ RK[7][2];
        state[3] = S[state[3]] ^ RK[7][3];
        state[4] = S[state[4]] ^ RK[7][4];
        state[5] = S[state[5]] ^ RK[7][5];
        state[6] = S[state[6]] ^ RK[7][6];
        state[7] = S[state[7]] ^ RK[7][7];
        state[8] = S[state[8]] ^ RK[7][8];
        state[9] = S[state[9]] ^ RK[7][9];
        state[10]= S[state[10]]^ RK[7][10];
        state[11]= S[state[11]]^ RK[7][11];
        state[12]= S[state[12]]^ RK[7][12];
        state[13]= S[state[13]]^ RK[7][13];
        state[14]= S[state[14]]^ RK[7][14];
        state[15]= S[state[15]]^ RK[7][15];
        /* Round 8 */
        state[0] = S[state[0]] ^ RK[8][0];
        state[1] = S[state[1]] ^ RK[8][1];
        state[2] = S[state[2]] ^ RK[8][2];
        state[3] = S[state[3]] ^ RK[8][3];
        state[4] = S[state[4]] ^ RK[8][4];
        state[5] = S[state[5]] ^ RK[8][5];
        state[6] = S[state[6]] ^ RK[8][6];
        state[7] = S[state[7]] ^ RK[8][7];
        state[8] = S[state[8]] ^ RK[8][8];
        state[9] = S[state[9]] ^ RK[8][9];
        state[10]= S[state[10]]^ RK[8][10];
        state[11]= S[state[11]]^ RK[8][11];
        state[12]= S[state[12]]^ RK[8][12];
        state[13]= S[state[13]]^ RK[8][13];
        state[14]= S[state[14]]^ RK[8][14];
        state[15]= S[state[15]]^ RK[8][15];
        /* Round 9 */
        state[0] = S[state[0]] ^ RK[9][0];
        state[1] = S[state[1]] ^ RK[9][1];
        state[2] = S[state[2]] ^ RK[9][2];
        state[3] = S[state[3]] ^ RK[9][3];
        state[4] = S[state[4]] ^ RK[9][4];
        state[5] = S[state[5]] ^ RK[9][5];
        state[6] = S[state[6]] ^ RK[9][6];
        state[7] = S[state[7]] ^ RK[9][7];
        state[8] = S[state[8]] ^ RK[9][8];
        state[9] = S[state[9]] ^ RK[9][9];
        state[10]= S[state[10]]^ RK[9][10];
        state[11]= S[state[11]]^ RK[9][11];
        state[12]= S[state[12]]^ RK[9][12];
        state[13]= S[state[13]]^ RK[9][13];
        state[14]= S[state[14]]^ RK[9][14];
        state[15]= S[state[15]]^ RK[9][15];
        /* Round 10 */
        state[0] = S[state[0]] ^ RK[10][0];
        state[1] = S[state[1]] ^ RK[10][1];
        state[2] = S[state[2]] ^ RK[10][2];
        state[3] = S[state[3]] ^ RK[10][3];
        state[4] = S[state[4]] ^ RK[10][4];
        state[5] = S[state[5]] ^ RK[10][5];
        state[6] = S[state[6]] ^ RK[10][6];
        state[7] = S[state[7]] ^ RK[10][7];
        state[8] = S[state[8]] ^ RK[10][8];
        state[9] = S[state[9]] ^ RK[10][9];
        state[10]= S[state[10]]^ RK[10][10];
        state[11]= S[state[11]]^ RK[10][11];
        state[12]= S[state[12]]^ RK[10][12];
        state[13]= S[state[13]]^ RK[10][13];
        state[14]= S[state[14]]^ RK[10][14];
        state[15]= S[state[15]]^ RK[10][15];
        /* Round 11 (final) */
        state[0] = S[state[0]] ^ RK[11][0];
        state[1] = S[state[1]] ^ RK[11][1];
        state[2] = S[state[2]] ^ RK[11][2];
        state[3] = S[state[3]] ^ RK[11][3];
        state[4] = S[state[4]] ^ RK[11][4];
        state[5] = S[state[5]] ^ RK[11][5];
        state[6] = S[state[6]] ^ RK[11][6];
        state[7] = S[state[7]] ^ RK[11][7];
        state[8] = S[state[8]] ^ RK[11][8];
        state[9] = S[state[9]] ^ RK[11][9];
        state[10]= S[state[10]]^ RK[11][10];
        state[11]= S[state[11]]^ RK[11][11];
        state[12]= S[state[12]]^ RK[11][12];
        state[13]= S[state[13]]^ RK[11][13];
        state[14]= S[state[14]]^ RK[11][14];
        state[15]= S[state[15]]^ RK[11][15];

        /* output ciphertext */
        for (i = 0; i < 16; ++i)
            printf("%02x", state[i]);
        printf("\n\n");

        /* prepare prev for next block (CBC) */
        prev[0] = state[0];  prev[1] = state[1];  prev[2] = state[2];  prev[3] = state[3];
        prev[4] = state[4];  prev[5] = state[5];  prev[6] = state[6];  prev[7] = state[7];
        prev[8] = state[8];  prev[9] = state[9];  prev[10]= state[10]; prev[11]= state[11];
        prev[12]= state[12]; prev[13]= state[13]; prev[14]= state[14]; prev[15]= state[15];
    }

    /* case C : two blocks */
    printf("Case C (2 blocks):\n");
    {
        int bIdx;
        for (bIdx = 0; bIdx < 2; ++bIdx) {
            int state[16];
            /* XOR with previous ciphertext (or IV for first block) */
            state[0] = blocks[bIdx][0] ^ prev[0];
            state[1] = blocks[bIdx][1] ^ prev[1];
            state[2] = blocks[bIdx][2] ^ prev[2];
            state[3] = blocks[bIdx][3] ^ prev[3];
            state[4] = blocks[bIdx][4] ^ prev[4];
            state[5] = blocks[bIdx][5] ^ prev[5];
            state[6] = blocks[bIdx][6] ^ prev[6];
            state[7] = blocks[bIdx][7] ^ prev[7];
            state[8] = blocks[bIdx][8] ^ prev[8];
            state[9] = blocks[bIdx][9] ^ prev[9];
            state[10]= blocks[bIdx][10]^ prev[10];
            state[11]= blocks[bIdx][11]^ prev[11];
            state[12]= blocks[bIdx][12]^ prev[12];
            state[13]= blocks[bIdx][13]^ prev[13];
            state[14]= blocks[bIdx][14]^ prev[14];
            state[15]= blocks[bIdx][15]^ prev[15];

            /* 12 rounds – same unrolled code as above (copy‑paste) */
            state[0] = S[state[0]] ^ RK[0][0];  state[1] = S[state[1]] ^ RK[0][1];
            state[2] = S[state[2]] ^ RK[0][2];  state[3] = S[state[3]] ^ RK[0][3];
            state[4] = S[state[4]] ^ RK[0][4];  state[5] = S[state[5]] ^ RK[0][5];
            state[6] = S[state[6]] ^ RK[0][6];  state[7] = S[state[7]] ^ RK[0][7];
            state[8] = S[state[8]] ^ RK[0][8];  state[9] = S[state[9]] ^ RK[0][9];
            state[10]= S[state[10]]^ RK[0][10];state[11]= S[state[11]]^ RK[0][11];
            state[12]= S[state[12]]^ RK[0][12];state[13]= S[state[13]]^ RK[0][13];
            state[14]= S[state[14]]^ RK[0][14];state[15]= S[state[15]]^ RK[0][15];

            state[0] = S[state[0]] ^ RK[1][0];  state[1] = S[state[1]] ^ RK[1][1];
            state[2] = S[state[2]] ^ RK[1][2];  state[3] = S[state[3]] ^ RK[1][3];
            state[4] = S[state[4]] ^ RK[1][4];  state[5] = S[state[5]] ^ RK[1][5];
            state[6] = S[state[6]] ^ RK[1][6];  state[7] = S[state[7]] ^ RK[1][7];
            state[8] = S[state[8]] ^ RK[1][8];  state[9] = S[state[9]] ^ RK[1][9];
            state[10]= S[state[10]]^ RK[1][10];state[11]= S[state[11]]^ RK[1][11];
            state[12]= S[state[12]]^ RK[1][12];state[13]= S[state[13]]^ RK[1][13];
            state[14]= S[state[14]]^ RK[1][14];state[15]= S[state[15]]^ RK[1][15];

            state[0] = S[state[0]] ^ RK[2][0];  state[1] = S[state[1]] ^ RK[2][1];
            state[2] = S[state[2]] ^ RK[2][2];  state[3] = S[state[3]] ^ RK[2][3];
            state[4] = S[state[4]] ^ RK[2][4];  state[5] = S[state[5]] ^ RK[2][5];
            state[6] = S[state[6]] ^ RK[2][6];  state[7] = S[state[7]] ^ RK[2][7];
            state[8] = S[state[8]] ^ RK[2][8];  state[9] = S[state[9]] ^ RK[2][9];
            state[10]= S[state[10]]^ RK[2][10];state[11]= S[state[11]]^ RK[2][11];
            state[12]= S[state[12]]^ RK[2][12];state[13]= S[state[13]]^ RK[2][13];
            state[14]= S[state[14]]^ RK[2][14];state[15]= S[state[15]]^ RK[2][15];

            state[0] = S[state[0]] ^ RK[3][0];  state[1] = S[state[1]] ^ RK[3][1];
            state[2] = S[state[2]] ^ RK[3][2];  state[3] = S[state[3]] ^ RK[3][3];
            state[4] = S[state[4]] ^ RK[3][4];  state[5] = S[state[5]] ^ RK[3][5];
            state[6] = S[state[6]] ^ RK[3][6];  state[7] = S[state[7]] ^ RK[3][7];
            state[8] = S[state[8]] ^ RK[3][8];  state[9] = S[state[9]] ^ RK[3][9];
            state[10]= S[state[10]]^ RK[3][10];state[11]= S[state[11]]^ RK[3][11];
            state[12]= S[state[12]]^ RK[3][12];state[13]= S[state[13]]^ RK[3][13];
            state[14]= S[state[14]]^ RK[3][14];state[15]= S[state[15]]^ RK[3][15];

            state[0] = S[state[0]] ^ RK[4][0];  state[1] = S[state[1]] ^ RK[4][1];
            state[2] = S[state[2]] ^ RK[4][2];  state[3] = S[state[3]] ^ RK[4][3];
            state[4] = S[state[4]] ^ RK[4][4];  state[5] = S[state[5]] ^ RK[4][5];
            state[6] = S[state[6]] ^ RK[4][6];  state[7] = S[state[7]] ^ RK[4][7];
            state[8] = S[state[8]] ^ RK[4][8];  state[9] = S[state[9]] ^ RK[4][9];
            state[10]= S[state[10]]^ RK[4][10];state[11]= S[state[11]]^ RK[4][11];
            state[12]= S[state[12]]^ RK[4][12];state[13]= S[state[13]]^ RK[4][13];
            state[14]= S[state[14]]^ RK[4][14];state[15]= S[state[15]]^ RK[4][15];

            state[0] = S[state[0]] ^ RK[5][0];  state[1] = S[state[1]] ^ RK[5][1];
            state[2] = S[state[2]] ^ RK[5][2];  state[3] = S[state[3]] ^ RK[5][3];
            state[4] = S[state[4]] ^ RK[5][4];  state[5] = S[state[5]] ^ RK[5][5];
            state[6] = S[state[6]] ^ RK[5][6];  state[7] = S[state[7]] ^ RK[5][7];
            state[8] = S[state[8]] ^ RK[5][8];  state[9] = S[state[9]] ^ RK[5][9];
            state[10]= S[state[10]]^ RK[5][10];state[11]= S[state[11]]^ RK[5][11];
            state[12]= S[state[12]]^ RK[5][12];state[13]= S[state[13]]^ RK[5][13];
            state[14]= S[state[14]]^ RK[5][14];state[15]= S[state[15]]^ RK[5][15];

            state[0] = S[state[0]] ^ RK[6][0];  state[1] = S[state[1]] ^ RK[6][1];
            state[2] = S[state[2]] ^ RK[6][2];  state[3] = S[state[3]] ^ RK[6][3];
            state[4] = S[state[4]] ^ RK[6][4];  state[5] = S[state[5]] ^ RK[6][5];
            state[6] = S[state[6]] ^ RK[6][6];  state[7] = S[state[7]] ^ RK[6][7];
            state[8] = S[state[8]] ^ RK[6][8];  state[9] = S[state[9]] ^ RK[6][9];
            state[10]= S[state[10]]^ RK[6][10];state[11]= S[state[11]]^ RK[6][11];
            state[12]= S[state[12]]^ RK[6][12];state[13]= S[state[13]]^ RK[6][13];
            state[14]= S[state[14]]^ RK[6][14];state[15]= S[state[15]]^ RK[6][15];

            state[0] = S[state[0]] ^ RK[7][0];  state[1] = S[state[1]] ^ RK[7][1];
            state[2] = S[state[2]] ^ RK[7][2];  state[3] = S[state[3]] ^ RK[7][3];
            state[4] = S[state[4]] ^ RK[7][4];  state[5] = S[state[5]] ^ RK[7][5];
            state[6] = S[state[6]] ^ RK[7][6];  state[7] = S[state[7]] ^ RK[7][7];
            state[8] = S[state[8]] ^ RK[7][8];  state[9] = S[state[9]] ^ RK[7][9];
            state[10]= S[state[10]]^ RK[7][10];state[11]= S[state[11]]^ RK[7][11];
            state[12]= S[state[12]]^ RK[7][12];state[13]= S[state[13]]^ RK[7][13];
            state[14]= S[state[14]]^ RK[7][14];state[15]= S[state[15]]^ RK[7][15];

            state[0] = S[state[0]] ^ RK[8][0];  state[1] = S[state[1]] ^ RK[8][1];
            state[2] = S[state[2]] ^ RK[8][2];  state[3] = S[state[3]] ^ RK[8][3];
            state[4] = S[state[4]] ^ RK[8][4];  state[5] = S[state[5]] ^ RK[8][5];
            state[6] = S[state[6]] ^ RK[8][6];  state[7] = S[state[7]] ^ RK[8][7];
            state[8] = S[state[8]] ^ RK[8][8];  state[9] = S[state[9]] ^ RK[8][9];
            state[10]= S[state[10]]^ RK[8][10];state[11]= S[state[11]]^ RK[8][11];
            state[12]= S[state[12]]^ RK[8][12];state[13]= S[state[13]]^ RK[8][13];
            state[14]= S[state[14]]^ RK[8][14];state[15]= S[state[15]]^ RK[8][15];

            state[0] = S[state[0]] ^ RK[9][0];  state[1] = S[state[1]] ^ RK[9][1];
            state[2] = S[state[2]] ^ RK[9][2];  state[3] = S[state[3]] ^ RK[9][3];
            state[4] = S[state[4]] ^ RK[9][4];  state[5] = S[state[5]] ^ RK[9][5];
            state[6] = S[state[6]] ^ RK[9][6];  state[7] = S[state[7]] ^ RK[9][7];
            state[8] = S[state[8]] ^ RK[9][8];  state[9] = S[state[9]] ^ RK[9][9];
            state[10]= S[state[10]]^ RK[9][10];state[11]= S[state[11]]^ RK[9][11];
            state[12]= S[state[12]]^ RK[9][12];state[13]= S[state[13]]^ RK[9][13];
            state[14]= S[state[14]]^ RK[9][14];state[15]= S[state[15]]^ RK[9][15];

            state[0] = S[state[0]] ^ RK[10][0]; state[1] = S[state[1]] ^ RK[10][1];
            state[2] = S[state[2]] ^ RK[10][2]; state[3] = S[state[3]] ^ RK[10][3];
            state[4] = S[state[4]] ^ RK[10][4]; state[5] = S[state[5]] ^ RK[10][5];
            state[6] = S[state[6]] ^ RK[10][6]; state[7] = S[state[7]] ^ RK[10][7];
            state[8] = S[state[8]] ^ RK[10][8]; state[9] = S[state[9]] ^ RK[10][9];
            state[10]= S[state[10]]^ RK[10][10];state[11]= S[state[11]]^ RK[10][11];
            state[12]= S[state[12]]^ RK[10][12];state[13]= S[state[13]]^ RK[10][13];
            state[14]= S[state[14]]^ RK[10][14];state[15]= S[state[15]]^ RK[10][15];

            state[0] = S[state[0]] ^ RK[11][0]; state[1] = S[state[1]] ^ RK[11][1];
            state[2] = S[state[2]] ^ RK[11][2]; state[3] = S[state[3]] ^ RK[11][3];
            state[4] = S[state[4]] ^ RK[11][4]; state[5] = S[state[5]] ^ RK[11][5];
            state[6] = S[state[6]] ^ RK[11][6]; state[7] = S[state[7]] ^ RK[11][7];
            state[8] = S[state[8]] ^ RK[11][8]; state[9] = S[state[9]] ^ RK[11][9];
            state[10]= S[state[10]]^ RK[11][10];state[11]= S[state[11]]^ RK[11][11];
            state[12]= S[state[12]]^ RK[11][12];state[13]= S[state[13]]^ RK[11][13];
            state[14]= S[state[14]]^ RK[11][14];state[15]= S[state[15]]^ RK[11][15];

            /* output */
            for (i = 0; i < 16; ++i)
                printf("%02x", state[i]);
            printf("\n");

            /* update chaining value */
            prev[0] = state[0];  prev[1] = state[1];  prev[2] = state[2];  prev[3] = state[3];
            prev[4] = state[4];  prev[5] = state[5];  prev[6] = state[6];  prev[7] = state[7];
            prev[8] = state[8];  prev[9] = state[9];  prev[10]= state[10]; prev[11]= state[11];
            prev[12]= state[12]; prev[13]= state[13]; prev[14]= state[14]; prev[15]= state[15];
        }
        printf("\n");
    }

    return 0;
}
