#include <iostream>
#include <iomanip>

/* LLM input variant 7: reverse-adversarial */
int main()
{
    /* --------------------------------------------------------------
       Verbose AES‑256‑XTS implementation.
       All data are allocated on the heap (new/delete) and only
       int / float types are used.
       -------------------------------------------------------------- */

    /* ---------- 1.  Hard‑coded S‑box and Rcon (int arrays) ---------- */
    int sBox[256] = {
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

    int rCon[15] = {1,2,4,8,16,32,64,128,27,54,108,216,171,77,154};

    /* ---------- 2.  Generate edge‑case heavy inputs ----------------- */
    /* 2.1 Keys (256‑bit each) – descending byte values */
    int* keyA = new int[32];
    int* keyB = new int[32];
    for (int i = 0; i < 32; ++i) {
        keyA[i] = (0xFF - i) & 0xFF;          // 0xFF,0xFE,... descending
        keyB[i] = (0xFE - i) & 0xFF;          // 0xFE,0xFD,... descending offset
    }

    /* 2.2 Plaintext: length just under two blocks to stress cipher‑stealing */
    int plainLen = 31;                     // 1 full block (16) + 15‑byte tail (worst case)
    int* plainBuf = new int[plainLen];
    for (int i = 0; i < plainLen; ++i) {
        plainBuf[i] = (0xFF - i) & 0xFF;    // descending data pattern
    }

    /* 2.3 Sector number (tweak input) – descending 128‑bit value */
    int* sectorNum = new int[16];
    for (int i = 0; i < 16; ++i) {
        sectorNum[i] = (0xFF - i) & 0xFF;    // 0xFF,0xFE,... descending
    }

    /* ---------- 3.  Helper lambdas (still inside main) ------------- */
    /* 3.1 RotWord – rotates a 4‑byte word left by one byte */
    auto rotWord = [](int* w) {
        int tmp = w[0];
        w[0] = w[1];
        w[1] = w[2];
        w[2] = w[3];
        w[3] = tmp;
    };

    /* 3.2 SubWord – applies S‑box to each byte of a word */
    auto subWord = [&](int* w) {
        for (int i = 0; i < 4; ++i) {
            w[i] = sBox[w[i] & 0xFF];
        }
    };

    /* 3.3 Key expansion for a 256‑bit key (produces 15 round keys) */
    auto expandKey = [&](int* keySrc, int* roundKeyBuf) {
        /* Copy original key as round 0 */
        for (int i = 0; i < 32; ++i) {
            roundKeyBuf[i] = keySrc[i];
        }

        int bytesGenerated = 32;      // how many bytes of round keys we already have
        int rconIdx = 0;

        while (bytesGenerated < 240) {   // 15 * 16 = 240 bytes needed
            int temp[4];
            /* Take last 4 bytes */
            for (int i = 0; i < 4; ++i) {
                temp[i] = roundKeyBuf[bytesGenerated - 4 + i];
            }

            if ((bytesGenerated % 32) == 0) {
                rotWord(temp);
                subWord(temp);
                temp[0] ^= rCon[rconIdx];
                ++rconIdx;
            } else if ((bytesGenerated % 32) == 16) {
                subWord(temp);
            }

            /* XOR with word 32 bytes back */
            for (int i = 0; i < 4; ++i) {
                temp[i] ^= roundKeyBuf[bytesGenerated - 32 + i];
                roundKeyBuf[bytesGenerated + i] = temp[i];
            }
            bytesGenerated += 4;
        }
    };

    /* 3.4 AddRoundKey – XOR state with round key */
    auto addRoundKey = [&](int* state, int* rKey) {
        for (int i = 0; i < 16; ++i) {
            state[i] ^= rKey[i];
        }
    };

    /* 3.5 SubBytes – apply S‑box to every state byte */
    auto subBytes = [&](int* state) {
        for (int i = 0; i < 16; ++i) {
            state[i] = sBox[state[i] & 0xFF];
        }
    };

    /* 3.6 ShiftRows – row wise cyclic shift */
    auto shiftRows = [&](int* st) {
        int tmp;

        /* Row 1 – shift left by 1 */
        tmp = st[1];
        st[1]  = st[5];
        st[5]  = st[9];
        st[9]  = st[13];
        st[13] = tmp;

        /* Row 2 – shift left by 2 */
        tmp = st[2];
        st[2]  = st[10];
        st[10] = tmp;
        tmp = st[6];
        st[6]  = st[14];
        st[14] = tmp;

        /* Row 3 – shift left by 3 (right by 1) */
        tmp = st[15];
        st[15] = st[11];
        st[11] = st[7];
        st[7]  = st[3];
        st[3]  = tmp;
    };

    /* 3.7 Multiply by 2 in GF(2^8) */
    auto xtime = [](int x) {
        x <<= 1;
        if (x & 0x100) x ^= 0x11B;
        return x & 0xFF;
    };

    /* 3.8 MixColumns – expanded multi‑step version */
    auto mixColumns = [&](int* st) {
        for (int c = 0; c < 4; ++c) {
            int i0 = c * 4 + 0;
            int i1 = c * 4 + 1;
            int i2 = c * 4 + 2;
            int i3 = c * 4 + 3;

            int a0 = st[i0];
            int a1 = st[i1];
            int a2 = st[i2];
            int a3 = st[i3];

            int t = a0 ^ a1 ^ a2 ^ a3;

            int u = a0;
            int v = a0 ^ a1; v = xtime(v); st[i0] ^= v ^ t;
            u = a1; v = a1 ^ a2; v = xtime(v); st[i1] ^= v ^ t;
            u = a2; v = a2 ^ a3; v = xtime(v); st[i2] ^= v ^ t;
            u = a3; v = a3 ^ a0; v = xtime(v); st[i3] ^= v ^ t;
        }
    };

    /* 3.9 AES‑256 single block encryption (uses prepared round keys) */
    auto aesEncryptBlock = [&](int* inBlk, int* outBlk, int* roundKeys) {
        int state[16];
        for (int i = 0; i < 16; ++i) state[i] = inBlk[i];

        /* Initial round */
        addRoundKey(state, roundKeys);          // round 0

        int round = 1;
        while (round < 14) {
            subBytes(state);
            shiftRows(state);
            mixColumns(state);
            addRoundKey(state, roundKeys + round * 16);
            ++round;
        }

        /* Final round (no MixColumns) */
        subBytes(state);
        shiftRows(state);
        addRoundKey(state, roundKeys + 14 * 16);

        for (int i = 0; i < 16; ++i) outBlk[i] = state[i];
    };

    /* 3.10 GF(2^128) multiplication by x (alpha) – used for tweak update */
    auto mulAlpha = [&](int* tweak) {
        int carry = 0;
        for (int i = 15; i >= 0; --i) {
            int nextCarry = (tweak[i] & 0x80) ? 1 : 0;
            tweak[i] = ((tweak[i] << 1) & 0xFF) | carry;
            carry = nextCarry;
        }
        if (carry) tweak[0] ^= 0x87;            // reduction polynomial
    };

    /* ---------- 4.  Prepare round keys for both AES keys ------------ */
    int* roundKeysA = new int[240];   // for data encryption (keyA)
    int* roundKeysB = new int[240];   // for tweak generation (keyB)

    expandKey(keyA, roundKeysA);
    expandKey(keyB, roundKeysB);

    /* ---------- 5.  Compute initial tweak (AES‑ECB of sector number) - */
    int* tweak = new int[16];
    aesEncryptBlock(sectorNum, tweak, roundKeysB);   // tweak = E_K2(sector)

    /* ---------- 6.  XTS encryption (verbose, heap‑only) ------------ */
    int blockCount = plainLen / 16;            // full 16‑byte blocks
    int tailBytes  = plainLen % 16;            // leftover bytes

    int* cipherBuf = new int[plainLen];        // output buffer

    int blockIdx = 0;
    while (blockIdx < blockCount) {
        /* ----- 6.1  Load plaintext block ----- */
        int plainBlock[16];
        for (int i = 0; i < 16; ++i) {
            plainBlock[i] = plainBuf[blockIdx * 16 + i];
        }

        /* ----- 6.2  XOR with current tweak ----- */
        int xored[16];
        for (int i = 0; i < 16; ++i) {
            xored[i] = plainBlock[i] ^ tweak[i];
        }

        /* ----- 6.3  AES‑256 encrypt with keyA ----- */
        int encrypted[16];
        aesEncryptBlock(xored, encrypted, roundKeysA);

        /* ----- 6.4  XOR encrypted data with tweak again ----- */
        for (int i = 0; i < 16; ++i) {
            encrypted[i] ^= tweak[i];
            cipherBuf[blockIdx * 16 + i] = encrypted[i];
        }

        /* ----- 6.5  Update tweak for next block ----- */
        mulAlpha(tweak);

        ++blockIdx;
    }

    /* ---------- 7.  Handle final partial block (cipher‑stealing) ---- */
    if (tailBytes != 0) {
        /* 7.1  Encrypt the last full block again to obtain a temporary tweak */
        int lastFullIdx = (blockCount - 1) * 16;
        int lastFullPlain[16];
        for (int i = 0; i < 16; ++i) {
            lastFullPlain[i] = plainBuf[lastFullIdx + i];
        }

        int lastFullXor[16];
        for (int i = 0; i < 16; ++i) {
            lastFullXor[i] = lastFullPlain[i] ^ tweak[i];
        }

        int lastFullEnc[16];
        aesEncryptBlock(lastFullXor, lastFullEnc, roundKeysA);

        for (int i = 0; i < 16; ++i) {
            lastFullEnc[i] ^= tweak[i];
        }

        /* 7.2  Steal ciphertext bytes */
        for (int i = 0; i < tailBytes; ++i) {
            cipherBuf[blockCount * 16 + i] = lastFullEnc[i];
        }

        /* 7.3  Replace the tail of the previous ciphertext block */
        for (int i = 0; i < tailBytes; ++i) {
            cipherBuf[lastFullIdx + i] = plainBuf[blockCount * 16 + i];
        }
    }

    /* ---------- 8.  Print results (hex) --------------------------- */
    std::cout << "Plaintext (" << plainLen << " bytes):\n";
    for (int i = 0; i < plainLen; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (plainBuf[i] & 0xFF) << " ";
        if ((i + 1) % 16 == 0) std::cout << "\n";
    }
    if (plainLen % 16 != 0) std::cout << "\n";

    std::cout << "\nCiphertext (XTS‑AES‑256):\n";
    for (int i = 0; i < plainLen; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (cipherBuf[i] & 0xFF) << " ";
        if ((i + 1) % 16 == 0) std::cout << "\n";
    }
    if (plainLen % 16 != 0) std::cout << "\n";

    /* ---------- 9.  Clean up heap memory --------------------------- */
    delete[] keyA;
    delete[] keyB;
    delete[] plainBuf;
    delete[] sectorNum;
    delete[] roundKeysA;
    delete[] roundKeysB;
    delete[] tweak;
    delete[] cipherBuf;

    return 0;
}
