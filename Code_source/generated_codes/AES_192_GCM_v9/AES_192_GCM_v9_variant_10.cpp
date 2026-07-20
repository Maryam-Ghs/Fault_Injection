#include <iostream>
#include <iomanip>

/* LLM input variant 10: large-safe-stress */

int main()
{
    /*--- S‑box and multiplication tables (int, no const) ---*/
    int sbox[256] = {
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
    int rcon[10] = {1,2,4,8,16,32,64,128,27,54};

    int mul2[256];
    int mul3[256];
    for (int i = 0; i < 256; ++i) {
        int x = i << 1;
        mul2[i] = ((x ^ 0x11B) & 0xFF) * ((x & 0x100) != 0) + (x & 0xFF) * ((x & 0x100) == 0);
        mul3[i] = mul2[i] ^ i;
    }

    /*--- Key, IV and plaintext (stack arrays) ---*/
    int kbytes[24] = {
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
    };
    int ivbytes[12] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    int pbytes[16] = {
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
    };
    int cbytes[16];
    int tagbytes[16];

    /*--- Key schedule for AES‑192 (52 words = 208 bytes) ---*/
    int wk[52][4];
    for (int i = 0; i < 6; ++i) {
        for (int j = 0; j < 4; ++j) wk[i][j] = kbytes[4*i + j];
    }
    for (int i = 6; i < 52; ++i) {
        int tmp[4];
        for (int j = 0; j < 4; ++j) tmp[j] = wk[i-1][j];
        if (i % 6 == 0) {
            int t = tmp[0];
            tmp[0] = sbox[tmp[1]]; tmp[1] = sbox[tmp[2]];
            tmp[2] = sbox[tmp[3]]; tmp[3] = sbox[t];
            tmp[0] ^= rcon[(i/6)-1];
        }
        for (int j = 0; j < 4; ++j) wk[i][j] = wk[i-6][j] ^ tmp[j];
    }

    /*--- AES‑192 block encrypt (in‑place) ---*/
    auto aes_enc = [&](int in[16], int out[16]) {
        int st[16];
        for (int i = 0; i < 16; ++i) st[i] = in[i] ^ wk[i/4][i%4];
        for (int round = 1; round < 12; ++round) {
            for (int i = 0; i < 16; ++i) st[i] = sbox[st[i]];
            int tmp[16];
            for (int r = 0; r < 4; ++r) {
                tmp[0+r*4] = st[0+r*4];
                tmp[1+r*4] = st[5%16+r*4];
                tmp[2+r*4] = st[10%16+r*4];
                tmp[3+r*4] = st[15%16+r*4];
            }
            for (int c = 0; c < 4; ++c) {
                int a0 = tmp[0+4*c], a1 = tmp[1+4*c], a2 = tmp[2+4*c], a3 = tmp[3+4*c];
                int t = a0 ^ a1 ^ a2 ^ a3;
                st[0+4*c] = a0 ^ t ^ mul2[a0 ^ a1];
                st[1+4*c] = a1 ^ t ^ mul2[a1 ^ a2];
                st[2+4*c] = a2 ^ t ^ mul2[a2 ^ a3];
                st[3+4*c] = a3 ^ t ^ mul2[a3 ^ a0];
            }
            for (int i = 0; i < 16; ++i) st[i] ^= wk[round*4 + i/4][i%4];
        }
        /*--- final round ---*/
        for (int i = 0; i < 16; ++i) st[i] = sbox[st[i]];
        int tmp[16];
        for (int r = 0; r < 4; ++r) {
            tmp[0+r*4] = st[0+r*4];
            tmp[1+r*4] = st[5%16+r*4];
            tmp[2+r*4] = st[10%16+r*4];
            tmp[3+r*4] = st[15%16+r*4];
        }
        for (int i = 0; i < 16; ++i) out[i] = tmp[i] ^ wk[48 + i/4][i%4];
    };

    /*--- Stress parameters ---*/
    const int NUM_ITER = 1000;   // large but safe number of repetitions

    for (int iter = 0; iter < NUM_ITER; ++iter) {
        /* Vary IV slightly each iteration to keep input changing */
        ivbytes[11] = (0xFF + iter) & 0xFF;

        /*--- Compute hash subkey H = AES‑192(0^128) ---*/
        int zeroBlock[16] = {0};
        int hbytes[16];
        aes_enc(zeroBlock, hbytes);

        /*--- Counter mode encryption (single block) ---*/
        int ctr[16] = {0};
        for (int i = 0; i < 12; ++i) ctr[i] = ivbytes[i];
        ctr[15] = 1;                     // J0 = IV || 0^31 || 1
        int keystream[16];
        aes_enc(ctr, keystream);
        for (int i = 0; i < 16; ++i) cbytes[i] = pbytes[i] ^ keystream[i];

        /*--- GHASH (single block, no AAD) ---*/
        int y[16] = {0};
        int x[16];
        for (int i = 0; i < 16; ++i) x[i] = y[i] ^ cbytes[i];
        /*--- 128‑bit multiplication in GF(2^128) ---*/
        auto mul128 = [&](int a[16], int b[16], int r[16]) {
            int z[16] = {0};
            int v[16];
            for (int i = 0; i < 16; ++i) v[i] = a[i];
            for (int i = 0; i < 128; ++i) {
                int byteIdx = i >> 3;
                int bitIdx  = 7 - (i & 7);
                int mask = -((b[byteIdx] >> bitIdx) & 1);
                for (int j = 0; j < 16; ++j) z[j] ^= v[j] & mask;
                int carry = (v[0] >> 7) & 1;
                for (int j = 0; j < 15; ++j) v[j] = ((v[j] << 1) & 0xFF) | ((v[j+1] >> 7) & 1);
                v[15] = ((v[15] << 1) & 0xFF) ^ (carry * 0xE1);
            }
            for (int i = 0; i < 16; ++i) r[i] = z[i];
        };
        mul128(x, hbytes, y);

        /*--- Tag = AES‑192(J0) xor Y ---*/
        int j0[16];
        for (int i = 0; i < 12; ++i) j0[i] = ivbytes[i];
        for (int i = 12; i < 15; ++i) j0[i] = 0;
        j0[15] = 1;
        int ekj0[16];
        aes_enc(j0, ekj0);
        for (int i = 0; i < 16; ++i) tagbytes[i] = ekj0[i] ^ y[i];
    }

    /*--- Output ciphertext and tag from the final iteration ---*/
    std::cout << "Ciphertext: ";
    for (int i = 0; i < 16; ++i)
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (cbytes[i] & 0xFF);
    std::cout << "\nTag:        ";
    for (int i = 0; i < 16; ++i)
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (tagbytes[i] & 0xFF);
    std::cout << std::dec << std::endl;
    return 0;
}
