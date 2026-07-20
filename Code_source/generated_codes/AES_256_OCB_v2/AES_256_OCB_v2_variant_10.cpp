#include <iostream>

/* LLM input variant 10: large-safe-stress */

int main()
{
    /* ---------- S‑box (plain int array, no const) ---------- */
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

    /* ---------- Rcon (plain int array) ---------- */
    int rcon[15] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36,
                    0x6C,0xD8,0xAB,0x4D,0x9A};

    /* ---------- Helper lambda for xtime (GF(2^8) multiply by 2) ---------- */
    auto xtime = [&](int x)->int{
        return ((x<<1) ^ ((x>>7)&1)*0x1b) & 0xFF;
    };

    /* ---------- Fixed 256‑bit key (32 bytes) ---------- */
    int key[32] = {
        0x60,0x3D,0xEB,0x10,0x15,0xCA,0x71,0xBE,
        0x2B,0x73,0xAE,0xF0,0x85,0x7D,0x77,0x81,
        0x1F,0x35,0x2C,0x07,0x3B,0x61,0x08,0xD7,
        0x2D,0x98,0x10,0xA3,0x09,0x14,0xDF,0xF4
    };

    /* ---------- Allocate key schedule (60 words = 240 bytes) ---------- */
    int* w = new int[60];

    /* ---------- Load first 8 words from key ---------- */
    int i = 0;
    while(i < 8){
        w[i] = (key[4*i]   <<24) |
               (key[4*i+1] <<16) |
               (key[4*i+2] <<8 ) |
               (key[4*i+3]      );
        i = i + 1;
    }

    /* ---------- Key expansion (manual loop, but unrolled per step) ---------- */
    while(i < 60){
        int temp = w[i-1];
        if(i % 8 == 0){
            /* RotWord */
            temp = ((temp << 8) | ((temp>>24)&0xFF)) & 0xFFFFFFFF;
            /* SubWord */
            int b0 = sbox[(temp>>24)&0xFF];
            int b1 = sbox[(temp>>16)&0xFF];
            int b2 = sbox[(temp>>8 )&0xFF];
            int b3 = sbox[(temp    )&0xFF];
            temp = (b0<<24)|(b1<<16)|(b2<<8)|b3;
            /* Rcon */
            temp = temp ^ (rcon[i/8-1] << 24);
        }else if(i % 8 == 4){
            /* SubWord */
            int b0 = sbox[(temp>>24)&0xFF];
            int b1 = sbox[(temp>>16)&0xFF];
            int b2 = sbox[(temp>>8 )&0xFF];
            int b3 = sbox[(temp    )&0xFF];
            temp = (b0<<24)|(b1<<16)|(b2<<8)|b3;
        }
        w[i] = w[i-8] ^ temp;
        i = i + 1;
    }

    /* ---------- Stress parameters ---------- */
    const int NUM_BLOCKS = 256;               // 256 blocks => 4096 bytes
    const int BLOCK_SIZE = 16;

    /* ---------- Plaintext (multiple blocks) ---------- */
    int pt[NUM_BLOCKS * BLOCK_SIZE];
    for(i = 0; i < NUM_BLOCKS * BLOCK_SIZE; ++i){
        pt[i] = i & 0xFF;                     // simple deterministic pattern
    }

    /* ---------- Nonce (single block, used as OCB nonce) ---------- */
    int nonce[BLOCK_SIZE] = {
        0x00,0x01,0x02,0x03,
        0x04,0x05,0x06,0x07,
        0x08,0x09,0x0A,0x0B,
        0x0C,0x0D,0x0E,0x0F
    };

    /* ---------- Helper lambda: AddRoundKey (xor 4 words) ---------- */
    auto addRoundKey = [&](int* state, int round){
        int k0 = w[round*4];
        int k1 = w[round*4+1];
        int k2 = w[round*4+2];
        int k3 = w[round*4+3];
        state[0]  ^= (k0>>24)&0xFF; state[1]  ^= (k0>>16)&0xFF;
        state[2]  ^= (k0>>8 )&0xFF; state[3]  ^= (k0    )&0xFF;
        state[4]  ^= (k1>>24)&0xFF; state[5]  ^= (k1>>16)&0xFF;
        state[6]  ^= (k1>>8 )&0xFF; state[7]  ^= (k1    )&0xFF;
        state[8]  ^= (k2>>24)&0xFF; state[9]  ^= (k2>>16)&0xFF;
        state[10] ^= (k2>>8 )&0xFF; state[11] ^= (k2    )&0xFF;
        state[12] ^= (k3>>24)&0xFF; state[13] ^= (k3>>16)&0xFF;
        state[14] ^= (k3>>8 )&0xFF; state[15] ^= (k3    )&0xFF;
    };

    /* ---------- Helper lambda: SubBytes (manual unrolled) ---------- */
    auto subBytes = [&](int* s){
        s[0] = sbox[s[0]];   s[1] = sbox[s[1]];
        s[2] = sbox[s[2]];   s[3] = sbox[s[3]];
        s[4] = sbox[s[4]];   s[5] = sbox[s[5]];
        s[6] = sbox[s[6]];   s[7] = sbox[s[7]];
        s[8] = sbox[s[8]];   s[9] = sbox[s[9]];
        s[10]= sbox[s[10]];  s[11]= sbox[s[11]];
        s[12]= sbox[s[12]];  s[13]= sbox[s[13]];
        s[14]= sbox[s[14]];  s[15]= sbox[s[15]];
    };

    /* ---------- Helper lambda: ShiftRows (manual) ---------- */
    auto shiftRows = [&](int* s){
        int t;
        t = s[1];  s[1]=s[5];  s[5]=s[9];  s[9]=s[13]; s[13]=t;
        t = s[2];  s[2]=s[10]; s[10]=t;
        t = s[6];  s[6]=s[14]; s[14]=t;
        t = s[3];  s[3]=s[15]; s[15]=s[11]; s[11]=s[7]; s[7]=t;
    };

    /* ---------- Helper lambda: MixColumns (manual, reordered ops) ---------- */
    auto mixColumns = [&](int* s){
        int a0,a1,a2,a3;
        a0 = s[0]; a1 = s[4]; a2 = s[8]; a3 = s[12];
        s[0] = xtime(a0) ^ (xtime(a1) ^ a1) ^ a2 ^ a3;
        s[4] = a0 ^ xtime(a1) ^ (xtime(a2) ^ a2) ^ a3;
        s[8] = a0 ^ a1 ^ xtime(a2) ^ (xtime(a3) ^ a3);
        s[12]= (xtime(a0) ^ a0) ^ a1 ^ a2 ^ xtime(a3);
        a0 = s[1]; a1 = s[5]; a2 = s[9]; a3 = s[13];
        s[1] = xtime(a0) ^ (xtime(a1) ^ a1) ^ a2 ^ a3;
        s[5] = a0 ^ xtime(a1) ^ (xtime(a2) ^ a2) ^ a3;
        s[9] = a0 ^ a1 ^ xtime(a2) ^ (xtime(a3) ^ a3);
        s[13]= (xtime(a0) ^ a0) ^ a1 ^ a2 ^ xtime(a3);
        a0 = s[2]; a1 = s[6]; a2 = s[10]; a3 = s[14];
        s[2] = xtime(a0) ^ (xtime(a1) ^ a1) ^ a2 ^ a3;
        s[6] = a0 ^ xtime(a1) ^ (xtime(a2) ^ a2) ^ a3;
        s[10]= a0 ^ a1 ^ xtime(a2) ^ (xtime(a3) ^ a3);
        s[14]= (xtime(a0) ^ a0) ^ a1 ^ a2 ^ xtime(a3);
        a0 = s[3]; a1 = s[7]; a2 = s[11]; a3 = s[15];
        s[3] = xtime(a0) ^ (xtime(a1) ^ a1) ^ a2 ^ a3;
        s[7] = a0 ^ xtime(a1) ^ (xtime(a2) ^ a2) ^ a3;
        s[11]= a0 ^ a1 ^ xtime(a2) ^ (xtime(a3) ^ a3);
        s[15]= (xtime(a0) ^ a0) ^ a1 ^ a2 ^ xtime(a3);
    };

    /* ---------- AES encrypt a single 16‑byte block (manual unrolled rounds) ---------- */
    auto aesEncryptBlock = [&](int* in, int* out){
        int state[16];
        int j = 0;
        while(j < 16){ state[j] = in[j]; j = j + 1; }
        addRoundKey(state,0);
        int round = 1;
        while(round <= 13){
            subBytes(state);
            shiftRows(state);
            mixColumns(state);
            addRoundKey(state,round);
            round = round + 1;
        }
        subBytes(state);
        shiftRows(state);
        addRoundKey(state,14);
        j = 0;
        while(j < 16){ out[j] = state[j]; j = j + 1; }
    };

    /* ---------- Compute L = AES(Zero block) (used for OCB) ---------- */
    int zero[BLOCK_SIZE];
    for(i = 0; i < BLOCK_SIZE; ++i) zero[i] = 0;
    int L[BLOCK_SIZE];
    aesEncryptBlock(zero, L);

    /* ---------- OCB offset for first block (using L) ---------- */
    int offset[BLOCK_SIZE];
    for(i = 0; i < BLOCK_SIZE; ++i) offset[i] = L[i];

    /* ---------- Process all blocks ---------- */
    int ct[NUM_BLOCKS * BLOCK_SIZE];
    int tmp[BLOCK_SIZE];
    int enc[BLOCK_SIZE];

    for(int blk = 0; blk < NUM_BLOCKS; ++blk){
        int base = blk * BLOCK_SIZE;
        for(i = 0; i < BLOCK_SIZE; ++i){
            tmp[i] = pt[base + i] ^ offset[i];
        }
        aesEncryptBlock(tmp, enc);
        for(i = 0; i < BLOCK_SIZE; ++i){
            ct[base + i] = enc[i] ^ offset[i];
        }
        // offset remains constant for this stress test (could be updated per OCB spec)
    }

    /* ---------- Compute checksum as XOR of all plaintext bytes ---------- */
    int checksum[BLOCK_SIZE] = {0};
    for(i = 0; i < NUM_BLOCKS * BLOCK_SIZE; ++i){
        checksum[i % BLOCK_SIZE] ^= pt[i];
    }

    /* ---------- Tag = AES( offset xor checksum ) xor offset ---------- */
    int tagIn[BLOCK_SIZE];
    for(i = 0; i < BLOCK_SIZE; ++i){
        tagIn[i] = offset[i] ^ checksum[i];
    }
    int tagEnc[BLOCK_SIZE];
    aesEncryptBlock(tagIn, tagEnc);
    int tag[BLOCK_SIZE];
    for(i = 0; i < BLOCK_SIZE; ++i){
        tag[i] = tagEnc[i] ^ offset[i];
    }

    /* ---------- Output ciphertext (all blocks) ---------- */
    std::cout << "Ciphertext ("
              << NUM_BLOCKS << " blocks):" << std::endl;
    for(int blk = 0; blk < NUM_BLOCKS; ++blk){
        std::cout << "Block " << blk << ": ";
        int base = blk * BLOCK_SIZE;
        for(i = 0; i < BLOCK_SIZE; ++i){
            std::cout << std::hex;
            if(ct[base + i] < 16) std::cout << '0';
            std::cout << ct[base + i];
            if(i != BLOCK_SIZE-1) std::cout << ' ';
        }
        std::cout << std::dec << std::endl;
    }

    std::cout << "Tag: ";
    for(i = 0; i < BLOCK_SIZE; ++i){
        std::cout << std::hex;
        if(tag[i] < 16) std::cout << '0';
        std::cout << tag[i];
        if(i != BLOCK_SIZE-1) std::cout << ' ';
    }
    std::cout << std::dec << std::endl;

    delete [] w;
    return 0;
}
