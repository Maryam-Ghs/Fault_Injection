/* LLM input variant 1: minimal-boundary */
#include <iostream>
#include <cstdlib>
#include <ctime>

int main()
{
    std::srand( static_cast<unsigned>( std::time( nullptr ) ) );

    /* -------------------------------------------------------------
       1. Generate deterministic minimal key, IV and plaintext
       ------------------------------------------------------------- */
    int *keyBytes = new int[16];
    int staticKey[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    for (int i = 0; i < 16; ++i) keyBytes[i] = staticKey[i];

    int *ivBlock = new int[16];
    int staticIV[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    for (int i = 0; i < 16; ++i) ivBlock[i] = staticIV[i];

    const int plainLen = 16;                     // single block (minimal non‑trivial)
    int *plainText = new int[plainLen];
    for (int i = 0; i < plainLen; ++i) plainText[i] = 0;   // all zeros

    /* -------------------------------------------------------------
       2. Build ARIA S‑boxes (hard‑coded, values fit into int)
       ------------------------------------------------------------- */
    int *S1 = new int[256];
    int *S2 = new int[256];
    int *S3 = new int[256];
    int *S4 = new int[256];

    // S‑box 1 (taken from the ARIA specification)
    int s1tbl[256] = {
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
    for (int i = 0; i < 256; ++i) {
        S1[i] = s1tbl[i];
        S2[i] = ((s1tbl[i] << 1) & 0xFF) | (s1tbl[i] >> 7);
        S3[i] = ((s1tbl[i] << 2) & 0xFF) | (s1tbl[i] >> 6);
        S4[i] = ((s1tbl[i] << 3) & 0xFF) | (s1tbl[i] >> 5);
    }

    /* -------------------------------------------------------------
       3. Helper lambdas (all live inside main → structure‑style rule)
       ------------------------------------------------------------- */
    auto rotlByte = [&](int v, int n) -> int {
        int left  = (v << n) & 0xFF;
        int right = v >> (8 - n);
        return left | right;
    };

    auto xorBlock = [&](int *a, int *b, int *out) {
        for (int i = 0; i < 16; ++i)
            out[i] = a[i] ^ b[i];
    };

    // Diffusion layer M0 (as defined in ARIA spec)
    auto diffusionM0 = [&](int *src, int *dst) {
        int t[16];
        for (int i = 0; i < 16; ++i) t[i] = src[i];
        dst[0] = t[3] ^ t[4] ^ t[6] ^ t[8] ^ t[9] ^ t[13] ^ t[14];
        dst[1] = t[2] ^ t[5] ^ t[7] ^ t[8] ^ t[10] ^ t[12] ^ t[15];
        dst[2] = t[1] ^ t[4] ^ t[6] ^ t[9] ^ t[11] ^ t[13] ^ t[15];
        dst[3] = t[0] ^ t[5] ^ t[7] ^ t[10] ^ t[12] ^ t[14] ^ t[15];
        dst[4] = t[0] ^ t[2] ^ t[5] ^ t[8] ^ t[11] ^ t[14] ^ t[15];
        dst[5] = t[1] ^ t[3] ^ t[4] ^ t[9] ^ t[10] ^ t[14] ^ t[15];
        dst[6] = t[0] ^ t[2] ^ t[7] ^ t[8] ^ t[13] ^ t[14] ^ t[15];
        dst[7] = t[1] ^ t[3] ^ t[6] ^ t[9] ^ t[12] ^ t[13] ^ t[15];
        dst[8] = t[0] ^ t[1] ^ t[4] ^ t[7] ^ t[10] ^ t[13] ^ t[15];
        dst[9] = t[0] ^ t[2] ^ t[5] ^ t[6] ^ t[11] ^ t[12] ^ t[15];
        dst[10]= t[1] ^ t[3] ^ t[5] ^ t[7] ^ t[8] ^ t[13] ^ t[15];
        dst[11]= t[2] ^ t[4] ^ t[6] ^ t[8] ^ t[9] ^ t[12] ^ t[15];
        dst[12]= t[0] ^ t[3] ^ t[5] ^ t[8] ^ t[10] ^ t[13] ^ t[14];
        dst[13]= t[1] ^ t[2] ^ t[6] ^ t[9] ^ t[11] ^ t[12] ^ t[14];
        dst[14]= t[0] ^ t[2] ^ t[4] ^ t[7] ^ t[11] ^ t[13] ^ t[15];
        dst[15]= t[1] ^ t[3] ^ t[5] ^ t[6] ^ t[10] ^ t[12] ^ t[14];
    };

    // One round of ARIA (substitution + diffusion)
    auto ariaRound = [&](int *blk, int *rk, int *out) {
        int tmp[16];
        // Substitution – use S‑boxes alternating per byte
        for (int i = 0; i < 16; ++i) {
            int b = blk[i] ^ rk[i];
            if (i % 4 == 0) tmp[i] = S1[b];
            else if (i % 4 == 1) tmp[i] = S2[b];
            else if (i % 4 == 2) tmp[i] = S3[b];
            else                tmp[i] = S4[b];
        }
        // Diffusion
        diffusionM0(tmp, out);
    };

    // Key schedule for 128‑bit key (produces 12 round keys)
    auto generateRoundKeys = [&](int *master, int **rounds) {
        // Simplified schedule: just rotate the master key per round
        for (int r = 0; r < 12; ++r) {
            for (int i = 0; i < 16; ++i) {
                int shifted = rotlByte(master[i], r + 1);
                rounds[r][i] = shifted;
            }
        }
    };

    // ARIA block encryption (12 rounds)
    auto ariaEncryptBlock = [&](int *blk, int **rk, int *out) {
        int state[16];
        for (int i = 0; i < 16; ++i) state[i] = blk[i];

        // 12 rounds – alternate forward / reverse diffusion (simplified)
        for (int r = 0; r < 12; ++r) {
            int next[16];
            ariaRound(state, rk[r], next);
            for (int i = 0; i < 16; ++i) state[i] = next[i];
        }
        // Final XOR with last round key
        xorBlock(state, rk[11], out);
    };

    // CFB‑mode encryption (full plaintext length)
    auto cfbEncrypt = [&](int *pt, int ptLen, int *iv, int **rk, int *ct) {
        int *prev = new int[16];
        for (int i = 0; i < 16; ++i) prev[i] = iv[i];

        int blockCnt = ptLen / 16;
        for (int b = 0; b < blockCnt; ++b) {
            int keystream[16];
            ariaEncryptBlock(prev, rk, keystream);

            // XOR plaintext block with keystream
            for (int i = 0; i < 16; ++i) {
                ct[b * 16 + i] = pt[b * 16 + i] ^ keystream[i];
            }
            // New feedback block = ciphertext block
            for (int i = 0; i < 16; ++i) prev[i] = ct[b * 16 + i];
        }
        delete[] prev;
    };

    /* -------------------------------------------------------------
       4. Allocate round‑key storage and generate them
       ------------------------------------------------------------- */
    int **roundKeys = new int*[12];
    for (int r = 0; r < 12; ++r) roundKeys[r] = new int[16];
    generateRoundKeys(keyBytes, roundKeys);

    /* -------------------------------------------------------------
       5. Perform CFB encryption
       ------------------------------------------------------------- */
    int *cipherText = new int[plainLen];
    cfbEncrypt(plainText, plainLen, ivBlock, roundKeys, cipherText);

    /* -------------------------------------------------------------
       6. Print key, IV, plaintext and ciphertext (hex format)
       ------------------------------------------------------------- */
    auto printHex = [&](const char *title, int *buf, int len) {
        std::cout << title;
        for (int i = 0; i < len; ++i) {
            int hi = (buf[i] >> 4) & 0xF;
            int lo = buf[i] & 0xF;
            std::cout << std::hex << (hi < 10 ? hi + '0' : hi - 10 + 'a')
                      << (lo < 10 ? lo + '0' : lo - 10 + 'a');
        }
        std::cout << std::dec << std::endl;
    };

    printHex("Key       : ", keyBytes, 16);
    printHex("IV        : ", ivBlock, 16);
    printHex("Plaintext : ", plainText, plainLen);
    printHex("Ciphertext: ", cipherText, plainLen);

    /* -------------------------------------------------------------
       7. Clean up heap allocations
       ------------------------------------------------------------- */
    delete[] keyBytes;
    delete[] ivBlock;
    delete[] plainText;
    delete[] cipherText;
    delete[] S1;
    delete[] S2;
    delete[] S3;
    delete[] S4;
    for (int r = 0; r < 12; ++r) delete[] roundKeys[r];
    delete[] roundKeys;

    return 0;
}
