#include <iostream>
#include <vector>

/* LLM input variant 9: medium-deterministic-random */

int main()
{
    // ------------------------------------------------------------
    // 1.  Helper tables (S‑box, Rcon) – use int, no const, no unsigned
    // ------------------------------------------------------------
    std::vector<int> sbox(256);
    std::vector<int> rcon(15);
    // S‑box values (partial, full table would be 256 entries – abbreviated here)
    int sb[256] = {
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
    for (int i = 0; i < 256; ++i) sbox[i] = sb[i];

    // Rcon (only first 15 needed for AES‑256)
    int rc[15] = {1,2,4,8,16,32,64,128,27,54,108,216,171,77,154};
    for (int i = 0; i < 15; ++i) rcon[i] = rc[i];

    // ------------------------------------------------------------
    // 2.  Deterministic pseudo‑random data generation (key, nonce, plaintext)
    // ------------------------------------------------------------
    // Simple linear congruential generator with fixed seed
    unsigned int lcg_state = 0xDEADBEEF;
    auto lcg_next = [&]() -> int {
        lcg_state = lcg_state * 1664525u + 1013904223u;
        return (lcg_state >> 24) & 0xFF;
    };

    // 256‑bit key (32 bytes)
    std::vector<int> key(32);
    for (int i = 0; i < 32; ++i) key[i] = lcg_next();

    // 96‑bit nonce (12 bytes) – OCB uses a 12‑byte nonce
    std::vector<int> nonce(12);
    for (int i = 0; i < 12; ++i) nonce[i] = lcg_next();

    // Plaintext: 6 blocks (96 bytes) – medium size deterministic data
    const int blockCount = 6;
    std::vector<int> plain(blockCount * 16);
    for (int i = 0; i < blockCount * 16; ++i) plain[i] = lcg_next();

    // ------------------------------------------------------------
    // 3.  Helper lambdas – GF(2^8) multiplication (xtime) and
    //     round transformations (manual unrolling, reordered ops)
    // ------------------------------------------------------------
    auto xtime = [](int x) -> int {
        int hi = (x & 0x80) ? 0x1B : 0;
        return ((x << 1) & 0xFF) ^ hi;
    };

    // SubBytes – fully unrolled
    auto subBytes = [&](std::vector<int>& st) {
        st[0] = sbox[st[0]];  st[1] = sbox[st[1]];  st[2] = sbox[st[2]];  st[3] = sbox[st[3]];
        st[4] = sbox[st[4]];  st[5] = sbox[st[5]];  st[6] = sbox[st[6]];  st[7] = sbox[st[7]];
        st[8] = sbox[st[8]];  st[9] = sbox[st[9]];  st[10]= sbox[st[10]]; st[11]= sbox[st[11]];
        st[12]= sbox[st[12]]; st[13]= sbox[st[13]]; st[14]= sbox[st[14]]; st[15]= sbox[st[15]];
    };

    // ShiftRows – manual, reordered index calculations
    auto shiftRows = [&](std::vector<int>& st) {
        int t;
        t = st[1];  st[1] = st[5];  st[5] = st[9];  st[9] = st[13];  st[13] = t;
        t = st[2];  st[2] = st[10];  st[10] = t;
        t = st[6];  st[6] = st[14];  st[14] = t;
        t = st[3];  st[3] = st[15];  st[15] = st[11];  st[11] = st[7];  st[7] = t;
    };

    // MixColumns – fully unrolled, reordered arithmetic
    auto mixColumns = [&](std::vector<int>& st) {
        int a0 = st[0], a1 = st[1], a2 = st[2], a3 = st[3];
        int a4 = st[4], a5 = st[5], a6 = st[6], a7 = st[7];
        int a8 = st[8], a9 = st[9], a10= st[10],a11= st[11];
        int a12= st[12],a13= st[13],a14= st[14],a15= st[15];

        int b0 = xtime(a0) ^ xtime(a1) ^ a1 ^ a2 ^ a3;
        int b1 = a0 ^ xtime(a1) ^ xtime(a2) ^ a2 ^ a3;
        int b2 = a0 ^ a1 ^ xtime(a2) ^ xtime(a3) ^ a3;
        int b3 = xtime(a0) ^ a0 ^ a1 ^ a2 ^ xtime(a3);

        int b4 = xtime(a4) ^ xtime(a5) ^ a5 ^ a6 ^ a7;
        int b5 = a4 ^ xtime(a5) ^ xtime(a6) ^ a6 ^ a7;
        int b6 = a4 ^ a5 ^ xtime(a6) ^ xtime(a7) ^ a7;
        int b7 = xtime(a4) ^ a4 ^ a5 ^ a6 ^ xtime(a7);

        int b8 = xtime(a8) ^ xtime(a9) ^ a9 ^ a10 ^ a11;
        int b9 = a8 ^ xtime(a9) ^ xtime(a10) ^ a10 ^ a11;
        int b10= a8 ^ a9 ^ xtime(a10) ^ xtime(a11) ^ a11;
        int b11= xtime(a8) ^ a8 ^ a9 ^ a10 ^ xtime(a11);

        int b12= xtime(a12)^ xtime(a13)^ a13 ^ a14 ^ a15;
        int b13= a12 ^ xtime(a13)^ xtime(a14)^ a14 ^ a15;
        int b14= a12 ^ a13 ^ xtime(a14)^ xtime(a15)^ a15;
        int b15= xtime(a12)^ a12 ^ a13 ^ a14 ^ xtime(a15);

        st[0]=b0;  st[1]=b1;  st[2]=b2;  st[3]=b3;
        st[4]=b4;  st[5]=b5;  st[6]=b6;  st[7]=b7;
        st[8]=b8;  st[9]=b9;  st[10]=b10;st[11]=b11;
        st[12]=b12;st[13]=b13;st[14]=b14;st[15]=b15;
    };

    // AddRoundKey – unrolled
    auto addRoundKey = [&](std::vector<int>& st, const std::vector<int>& rk, int offset) {
        st[0] ^= rk[offset+0];  st[1] ^= rk[offset+1];
        st[2] ^= rk[offset+2];  st[3] ^= rk[offset+3];
        st[4] ^= rk[offset+4];  st[5] ^= rk[offset+5];
        st[6] ^= rk[offset+6];  st[7] ^= rk[offset+7];
        st[8] ^= rk[offset+8];  st[9] ^= rk[offset+9];
        st[10]^= rk[offset+10]; st[11]^= rk[offset+11];
        st[12]^= rk[offset+12]; st[13]^= rk[offset+13];
        st[14]^= rk[offset+14]; st[15]^= rk[offset+15];
    };

    // ------------------------------------------------------------
    // 4.  Key expansion for AES‑256 (14 rounds → 15 round keys)
    // ------------------------------------------------------------
    std::vector<int> roundKeys(15 * 16); // 15 * 16 bytes
    for (int i = 0; i < 32; ++i) roundKeys[i] = key[i];

    auto getWord = [&](int idx) -> std::vector<int> {
        std::vector<int> w(4);
        w[0] = roundKeys[idx*4+0];
        w[1] = roundKeys[idx*4+1];
        w[2] = roundKeys[idx*4+2];
        w[3] = roundKeys[idx*4+3];
        return w;
    };

    auto setWord = [&](int idx, const std::vector<int>& w) {
        roundKeys[idx*4+0] = w[0];
        roundKeys[idx*4+1] = w[1];
        roundKeys[idx*4+2] = w[2];
        roundKeys[idx*4+3] = w[3];
    };

    int i = 8; // we already have 8 words
    while (i < 60) { // 60 words = 240 bytes for AES‑256
        std::vector<int> temp = getWord(i-1);
        if (i % 8 == 0) {
            int t = temp[0];
            temp[0] = temp[1];
            temp[1] = temp[2];
            temp[2] = temp[3];
            temp[3] = t;
            temp[0] = sbox[temp[0]];
            temp[1] = sbox[temp[1]];
            temp[2] = sbox[temp[2]];
            temp[3] = sbox[temp[3]];
            temp[0] ^= rcon[(i/8)-1];
        } else if (i % 8 == 4) {
            temp[0] = sbox[temp[0]];
            temp[1] = sbox[temp[1]];
            temp[2] = sbox[temp[2]];
            temp[3] = sbox[temp[3]];
        }
        std::vector<int> prev = getWord(i-8);
        std::vector<int> neww(4);
        neww[0] = prev[0] ^ temp[0];
        neww[1] = prev[1] ^ temp[1];
        neww[2] = prev[2] ^ temp[2];
        neww[3] = prev[3] ^ temp[3];
        setWord(i, neww);
        ++i;
    }

    // ------------------------------------------------------------
    // 5.  AES‑256 block encrypt (one 16‑byte block)
    // ------------------------------------------------------------
    auto aesEncryptBlock = [&](const std::vector<int>& in, std::vector<int>& out) {
        std::vector<int> st(16);
        for (int j = 0; j < 16; ++j) st[j] = in[j];
        addRoundKey(st, roundKeys, 0);
        for (int r = 1; r <= 13; ++r) {
            subBytes(st);
            shiftRows(st);
            mixColumns(st);
            addRoundKey(st, roundKeys, r*16);
        }
        subBytes(st);
        shiftRows(st);
        addRoundKey(st, roundKeys, 14*16);
        for (int j = 0; j < 16; ++j) out[j] = st[j];
    };

    // ------------------------------------------------------------
    // 6.  OCB mode (encrypt + tag) – simplified, single‑pass
    // ------------------------------------------------------------
    auto doubleBlock = [&](std::vector<int>& blk) {
        int carry = 0;
        for (int j = 15; j >= 0; --j) {
            int newCarry = (blk[j] & 0x80) ? 1 : 0;
            blk[j] = ((blk[j] << 1) & 0xFF) ^ carry;
            carry = newCarry;
        }
        if (carry) blk[15] ^= 0x87;
    };

    std::vector<int> zeroBlock(16,0), Lstar(16);
    aesEncryptBlock(zeroBlock, Lstar);
    std::vector<int> L = Lstar;
    doubleBlock(L);

    std::vector<std::vector<int>> offsets(blockCount+1, std::vector<int>(16,0));
    std::vector<int> nonceBlock(16,0);
    for (int j = 0; j < 12; ++j) nonceBlock[j] = nonce[j];
    aesEncryptBlock(nonceBlock, offsets[0]);

    for (int i = 1; i <= blockCount; ++i) {
        offsets[i] = offsets[i-1];
        for (int j = 0; j < 16; ++j) offsets[i][j] ^= L[j];
    }

    std::vector<int> ciphertext(blockCount * 16);
    std::vector<int> sumTag(16,0);
    for (int blk = 0; blk < blockCount; ++blk) {
        std::vector<int> ptBlock(16);
        for (int j = 0; j < 16; ++j) ptBlock[j] = plain[blk*16 + j];
        for (int j = 0; j < 16; ++j) ptBlock[j] ^= offsets[blk][j];
        std::vector<int> encTmp(16);
        aesEncryptBlock(ptBlock, encTmp);
        for (int j = 0; j < 16; ++j) {
            int ct = encTmp[j] ^ offsets[blk][j];
            ciphertext[blk*16 + j] = ct;
            sumTag[j] ^= ct;
        }
    }

    std::vector<int> tagIn(16);
    for (int j = 0; j < 16; ++j) tagIn[j] = sumTag[j] ^ offsets[blockCount][j];
    std::vector<int> tag(16);
    aesEncryptBlock(tagIn, tag);

    // ------------------------------------------------------------
    // 7.  Decryption (verify we recover original plaintext)
    // ------------------------------------------------------------
    std::vector<int> recovered(blockCount * 16);
    for (int blk = 0; blk < blockCount; ++blk) {
        std::vector<int> ctBlock(16);
        for (int j = 0; j < 16; ++j) ctBlock[j] = ciphertext[blk*16 + j];
        for (int j = 0; j < 16; ++j) ctBlock[j] ^= offsets[blk][j];
        std::vector<int> decTmp(16);
        aesEncryptBlock(ctBlock, decTmp); // placeholder decryption
        for (int j = 0; j < 16; ++j) {
            int pt = decTmp[j] ^ offsets[blk][j];
            recovered[blk*16 + j] = pt;
        }
    }

    // ------------------------------------------------------------
    // 8.  Output results (hex style, one line per entity)
    // ------------------------------------------------------------
    auto printHex = [&](const std::vector<int>& data, const char* title) {
        std::cout << title << ": ";
        for (size_t i = 0; i < data.size(); ++i) {
            int v = data[i] & 0xFF;
            if (v < 16) std::cout << '0';
            std::cout << std::hex << v;
        }
        std::cout << std::dec << "\n";
    };

    printHex(key, "Key");
    printHex(nonce, "Nonce");
    printHex(plain, "Plaintext");
    printHex(ciphertext, "Ciphertext");
    printHex(tag, "Tag");
    printHex(recovered, "Recovered");

    return 0;
}
