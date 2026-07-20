#include <cstdio>
#include <vector>
#include <cstdlib>
/* LLM input variant 9: medium-deterministic-random */

int main() {
    /*--------------------------------------------------------------
       Generate deterministic pseudo‑random key, nonce, plaintext and associated data.
    --------------------------------------------------------------*/
    static unsigned int prng_state = 0x12345678;
    auto pseudo_rand = [&]() -> int {
        prng_state = prng_state * 1664525u + 1013904223u;
        return (prng_state >> 24) & 0xFF;
    };

    const int blockSize = 16;               // AES block size (bytes)
    const int tagSize   = 16;               // Authentication tag length
    const int L         = 2;                // Length field size for CCM
    const int M         = tagSize;          // Tag size for CCM

    // 256‑bit key (32 bytes)
    std::vector<int> key(32);
    for (int i = 0; i < 32; ++i) key[i] = pseudo_rand();

    // Nonce (12 bytes, typical for CCM with L=2)
    std::vector<int> nonce(12);
    for (int i = 0; i < 12; ++i) nonce[i] = pseudo_rand();

    // Plaintext (medium‑sized deterministic array)
    int msgLen = 128;                       // length in bytes
    std::vector<int> plain(msgLen);
    for (int i = 0; i < msgLen; ++i) plain[i] = pseudo_rand();

    // Associated data (optional, here 32 bytes)
    std::vector<int> assoc(32);
    for (int i = 0; i < 32; ++i) assoc[i] = pseudo_rand();

    /*--------------------------------------------------------------
       AES‑256 tables (S‑box, Rcon) – all stored as int vectors.
    --------------------------------------------------------------*/
    std::vector<int> sbox = {
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

    std::vector<int> rcon = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36};

    /*--------------------------------------------------------------
       Key expansion for AES‑256 (14 rounds, 240‑byte round key).
    --------------------------------------------------------------*/
    const int Nr = 14;
    std::vector<int> roundKey((Nr + 1) * blockSize);
    for (int i = 0; i < 32; ++i) roundKey[i] = key[i];

    int generated = 32;                     // bytes already generated
    int rconIdx   = 0;

    while (generated < (Nr + 1) * blockSize) {
        // take previous 4‑byte word
        int t0 = roundKey[generated - 4];
        int t1 = roundKey[generated - 3];
        int t2 = roundKey[generated - 2];
        int t3 = roundKey[generated - 1];

        // rotate left
        int tmp = t0; t0 = t1; t1 = t2; t2 = t3; t3 = tmp;

        // apply S‑box
        t0 = sbox[t0]; t1 = sbox[t1]; t2 = sbox[t2]; t3 = sbox[t3];

        // XOR with Rcon on the first byte
        t0 ^= rcon[rconIdx++];
        // first new word
        roundKey[generated]     = roundKey[generated - 32] ^ t0;
        roundKey[generated + 1] = roundKey[generated - 31] ^ t1;
        roundKey[generated + 2] = roundKey[generated - 30] ^ t2;
        roundKey[generated + 3] = roundKey[generated - 29] ^ t3;
        generated += 4;

        // next three words (simple XOR with word 8 bytes back)
        for (int j = 0; j < 3; ++j) {
            int idx = generated - 4;
            roundKey[generated]     = roundKey[generated - 32] ^ roundKey[idx];
            roundKey[generated + 1] = roundKey[generated - 31] ^ roundKey[idx + 1];
            roundKey[generated + 2] = roundKey[generated - 30] ^ roundKey[idx + 2];
            roundKey[generated + 3] = roundKey[generated - 29] ^ roundKey[idx + 3];
            generated += 4;
        }
    }

    /*--------------------------------------------------------------
       AES block encryption (lambda, uses only int arithmetic).
    --------------------------------------------------------------*/
    auto encryptBlock = [&](const std::vector<int> &in, std::vector<int> &out) {
        int state[16];
        for (int i = 0; i < 16; ++i) state[i] = in[i];

        // Initial AddRoundKey
        for (int i = 0; i < 16; ++i) state[i] ^= roundKey[i];

        // Rounds 1 … 13
        for (int round = 1; round < Nr; ++round) {
            // SubBytes
            for (int i = 0; i < 16; ++i) state[i] = sbox[state[i]];

            // ShiftRows (column‑major layout)
            int tmp[16];
            tmp[0] = state[0];  tmp[4] = state[4];  tmp[8]  = state[8];  tmp[12] = state[12];
            tmp[1] = state[5];  tmp[5] = state[9];  tmp[9]  = state[13]; tmp[13] = state[1];
            tmp[2] = state[10]; tmp[6] = state[14]; tmp[10] = state[2];  tmp[14] = state[6];
            tmp[3] = state[15]; tmp[7] = state[3];  tmp[11] = state[7];  tmp[15] = state[11];
            for (int i = 0; i < 16; ++i) state[i] = tmp[i];

            // MixColumns
            for (int c = 0; c < 4; ++c) {
                int a0 = state[4 * c];
                int a1 = state[4 * c + 1];
                int a2 = state[4 * c + 2];
                int a3 = state[4 * c + 3];

                int r0 = ((a0 << 1) & 0xFE) ^ ((a0 >> 7) & 1) * 0x1B;
                int r1 = ((a1 << 1) & 0xFE) ^ ((a1 >> 7) & 1) * 0x1B;
                int r2 = ((a2 << 1) & 0xFE) ^ ((a2 >> 7) & 1) * 0x1B;
                int r3 = ((a3 << 1) & 0xFE) ^ ((a3 >> 7) & 1) * 0x1B;

                int b0 = r0 ^ a1 ^ a2 ^ r3 ^ a3;
                int b1 = a0 ^ r1 ^ a2 ^ a3 ^ r0;
                int b2 = a0 ^ a1 ^ r2 ^ a3 ^ r1;
                int b3 = a0 ^ a1 ^ a2 ^ r3 ^ r2;

                state[4 * c]     = b0 & 0xFF;
                state[4 * c + 1] = b1 & 0xFF;
                state[4 * c + 2] = b2 & 0xFF;
                state[4 * c + 3] = b3 & 0xFF;
            }

            // AddRoundKey
            int rkOff = round * blockSize;
            for (int i = 0; i < 16; ++i) state[i] ^= roundKey[rkOff + i];
        }

        // Final round (no MixColumns)
        for (int i = 0; i < 16; ++i) state[i] = sbox[state[i]];
        int tmp2[16];
        tmp2[0] = state[0];  tmp2[4] = state[4];  tmp2[8]  = state[8];  tmp2[12] = state[12];
        tmp2[1] = state[5];  tmp2[5] = state[9];  tmp2[9]  = state[13]; tmp2[13] = state[1];
        tmp2[2] = state[10]; tmp2[6] = state[14]; tmp2[10] = state[2];  tmp2[14] = state[6];
        tmp2[3] = state[15]; tmp2[7] = state[3];  tmp2[11] = state[7];  tmp2[15] = state[11];
        for (int i = 0; i < 16; ++i) state[i] = tmp2[i];

        // Final AddRoundKey
        int finalOff = Nr * blockSize;
        for (int i = 0; i < 16; ++i) out[i] = state[i] ^ roundKey[finalOff + i];
    };

    /*--------------------------------------------------------------
       CCM – build B0, run CBC‑MAC, encrypt with CTR, compute tag.
    --------------------------------------------------------------*/
    // ----- B0 ----------------------------------------------------
    std::vector<int> B0(16);
    int flags = ((assoc.size() > 0 ? (1 << 6) : 0)) | (((M - 2) / 2) << 3) | (L - 1);
    B0[0] = flags;
    for (int i = 0; i < (int)nonce.size(); ++i) B0[1 + i] = nonce[i];
    int lenTmp = msgLen;
    for (int i = 0; i < L; ++i) {
        B0[15 - i] = lenTmp & 0xFF;
        lenTmp >>= 8;
    }

    // ----- CBC‑MAC -----------------------------------------------
    std::vector<int> mac(16);
    encryptBlock(B0, mac);

    // Associated data (if any)
    if (!assoc.empty()) {
        // length encoding (2‑byte, big‑endian)
        std::vector<int> aLen(2);
        aLen[0] = (assoc.size() >> 8) & 0xFF;
        aLen[1] = assoc.size() & 0xFF;

        // first block: length + first part of data
        std::vector<int> blk(16, 0);
        int pos = 0;
        for (int i = 0; i < 2; ++i) blk[pos++] = aLen[i];
        for (int i = 0; i < 14 && pos < 16 && i < (int)assoc.size(); ++i) blk[pos++] = assoc[i];

        for (int i = 0; i < 16; ++i) blk[i] ^= mac[i];
        encryptBlock(blk, mac);

        // remaining associated data
        int off = 14;
        while (off < (int)assoc.size()) {
            std::vector<int> blk2(16, 0);
            for (int i = 0; i < 16 && off < (int)assoc.size(); ++i, ++off) blk2[i] = assoc[off];
            for (int i = 0; i < 16; ++i) blk2[i] ^= mac[i];
            encryptBlock(blk2, mac);
        }
    }

    // Plaintext blocks
    int ptOff = 0;
    while (ptOff < msgLen) {
        std::vector<int> blk(16, 0);
        for (int i = 0; i < 16 && ptOff < msgLen; ++i, ++ptOff) blk[i] = plain[ptOff];
        for (int i = 0; i < 16; ++i) blk[i] ^= mac[i];
        encryptBlock(blk, mac);
    }

    // Truncate MAC to tag size M
    std::vector<int> rawTag(M);
    for (int i = 0; i < M; ++i) rawTag[i] = mac[i];

    // ----- CTR encryption -----------------------------------------
    // Counter block with flags = L‑1, counter = 1
    std::vector<int> ctr0(16);
    ctr0[0] = L - 1;
    for (int i = 0; i < (int)nonce.size(); ++i) ctr0[1 + i] = nonce[i];
    ctr0[15] = 1;

    std::vector<int> cipher(msgLen);
    int cPos = 0;
    int blockIdx = 1;
    while (cPos < msgLen) {
        // Build counter for this block
        std::vector<int> ctr(16);
        ctr[0] = L - 1;
        for (int i = 0; i < (int)nonce.size(); ++i) ctr[1 + i] = nonce[i];
        int cnt = blockIdx;
        for (int i = 0; i < L; ++i) {
            ctr[15 - i] = cnt & 0xFF;
            cnt >>= 8;
        }

        std::vector<int> keystream(16);
        encryptBlock(ctr, keystream);

        for (int i = 0; i < 16 && cPos < msgLen; ++i, ++cPos)
            cipher[cPos] = plain[cPos] ^ keystream[i];

        ++blockIdx;
    }

    // ----- Tag encryption (S0) ------------------------------------
    std::vector<int> s0(16);
    encryptBlock(ctr0, s0);
    std::vector<int> tag(M);
    for (int i = 0; i < M; ++i) tag[i] = rawTag[i] ^ s0[i];

    /*--------------------------------------------------------------
       Helper to print vectors as hex strings.
    --------------------------------------------------------------*/
    auto printHex = [&](const std::vector<int> &v, const char *title) {
        printf("%s: ", title);
        for (size_t i = 0; i < v.size(); ++i) printf("%02X", v[i] & 0xFF);
        printf("\n");
    };

    // ----- Output -------------------------------------------------
    printHex(key,    "Key");
    printHex(nonce,  "Nonce");
    printHex(plain,  "Plaintext");
    printHex(cipher, "Ciphertext");
    printHex(tag,    "Tag");

    return 0;
}
