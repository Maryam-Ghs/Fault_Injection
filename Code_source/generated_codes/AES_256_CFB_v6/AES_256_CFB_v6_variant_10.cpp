#include <iostream>
#include <iomanip>

/* LLM input variant 10: large-safe-stress */

int main()
{
    // ---------- test vectors ----------
    int key[32] = {
        0x60,0x3d,0xeb,0x10,0x15,0xca,0x71,0xbe,
        0x2b,0x73,0xae,0xf0,0x85,0x7d,0x77,0x81,
        0x1f,0x35,0x2c,0x07,0x3b,0x61,0x08,0xd7,
        0x2d,0x98,0x10,0xa3,0x09,0x14,0xdf,0xf4
    };
    int iv[16] = {
        0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
        0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f
    };
    const int BLOCKS = 10;               // stress test with 10 blocks (160 bytes)
    int pt[BLOCKS * 16];
    for (int i = 0; i < BLOCKS * 16; ++i) {
        pt[i] = i & 0xFF;                 // deterministic pattern 0x00..0xFF cyclic
    }

    // ---------- S‑box ----------
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

    // ---------- Rcon ----------
    int rcon[15] = {
        0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36,
        0x6C,0xD8,0xAB,0x4D,0x9A
    };

    // ---------- helpers ----------
    auto mul2 = [&](int x)->int{
        int y = (x<<1) & 0xFF;
        y ^= ((x>>7)&1)*0x1B;
        return y;
    };
    auto mul3 = [&](int x)->int{
        return mul2(x) ^ x;
    };

    // ---------- key expansion ----------
    int Nb = 4, Nk = 8, Nr = 14;
    int* w = new int[(Nb*(Nr+1))*4];               // 240 bytes = 60 words * 4
    int i = 0;
    while(i < Nk*4){
        w[i] = key[i];
        i = i + 1;
    }
    i = Nk*4;
    while(i < Nb*(Nr+1)*4){
        int temp0 = w[i-4];
        int temp1 = w[i-3];
        int temp2 = w[i-2];
        int temp3 = w[i-1];
        // RotWord
        if(((i/4) % Nk) == 0){
            int t = temp0;
            temp0 = temp1; temp1 = temp2; temp2 = temp3; temp3 = t;
            // SubWord
            temp0 = sbox[temp0];
            temp1 = sbox[temp1];
            temp2 = sbox[temp2];
            temp3 = sbox[temp3];
            // Rcon
            temp0 ^= rcon[(i/4)/Nk];
        }else if(Nk>6 && ((i/4) % Nk) == 4){
            // SubWord
            temp0 = sbox[temp0];
            temp1 = sbox[temp1];
            temp2 = sbox[temp2];
            temp3 = sbox[temp3];
        }
        w[i]   = w[i - Nk*4]   ^ temp0;
        w[i+1] = w[i - Nk*4+1] ^ temp1;
        w[i+2] = w[i - Nk*4+2] ^ temp2;
        w[i+3] = w[i - Nk*4+3] ^ temp3;
        i = i + 4;
    }

    // ---------- AES round functions ----------
    auto subBytes = [&](int* st){
        int p = 0;
        while(p < 16){
            st[p] = sbox[st[p]];
            p = p + 1;
        }
    };
    auto shiftRows = [&](int* st){
        // row 1
        int a = st[1], b = st[5], c = st[9], d = st[13];
        st[1] = b; st[5] = c; st[9] = d; st[13] = a;
        // row 2
        a = st[2]; b = st[6]; c = st[10]; d = st[14];
        st[2] = c; st[6] = d; st[10] = a; st[14] = b;
        // row 3
        a = st[3]; b = st[7]; c = st[11]; d = st[15];
        st[3] = d; st[7] = a; st[11] = b; st[15] = c;
    };
    auto mixColumns = [&](int* st){
        int col = 0;
        while(col < 4){
            int i0 = col*4;
            int s0 = st[i0];
            int s1 = st[i0+1];
            int s2 = st[i0+2];
            int s3 = st[i0+3];
            int m0 = mul2(s0) ^ mul3(s1) ^ s2 ^ s3;
            int m1 = s0 ^ mul2(s1) ^ mul3(s2) ^ s3;
            int m2 = s0 ^ s1 ^ mul2(s2) ^ mul3(s3);
            int m3 = mul3(s0) ^ s1 ^ s2 ^ mul2(s3);
            st[i0]   = m0;
            st[i0+1] = m1;
            st[i0+2] = m2;
            st[i0+3] = m3;
            col = col + 1;
        }
    };
    auto addRoundKey = [&](int* st, int round){
        int off = round*16;
        int j = 0;
        while(j < 16){
            st[j] ^= w[off + j];
            j = j + 1;
        }
    };

    // ---------- encrypt one block ----------
    auto encBlock = [&](int* in, int* out){
        int* state = new int[16];
        int k = 0;
        while(k < 16){
            state[k] = in[k];
            k = k + 1;
        }
        addRoundKey(state,0);
        int r = 1;
        while(r < Nr){
            subBytes(state);
            shiftRows(state);
            mixColumns(state);
            addRoundKey(state,r);
            r = r + 1;
        }
        subBytes(state);
        shiftRows(state);
        addRoundKey(state,Nr);
        k = 0;
        while(k < 16){
            out[k] = state[k];
            k = k + 1;
        }
        delete[] state;
    };

    // ---------- CFB‑128 encryption over multiple blocks ----------
    int ct[BLOCKS * 16];
    int feedback[16];
    for (int j = 0; j < 16; ++j) feedback[j] = iv[j];

    int keystream[16];
    for (int blk = 0; blk < BLOCKS; ++blk) {
        encBlock(feedback, keystream);
        for (int j = 0; j < 16; ++j) {
            ct[blk*16 + j] = pt[blk*16 + j] ^ keystream[j];
        }
        // Update feedback with ciphertext block (CFB mode)
        for (int j = 0; j < 16; ++j) feedback[j] = ct[blk*16 + j];
    }

    // ---------- output ----------
    std::cout << "Ciphertext: ";
    for (int idx = 0; idx < BLOCKS * 16; ++idx) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (ct[idx] & 0xFF);
    }
    std::cout << std::dec << std::endl;

    // ---------- clean up ----------
    delete[] w;
    return 0;
}
