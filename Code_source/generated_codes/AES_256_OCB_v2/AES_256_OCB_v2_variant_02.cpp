#include <iostream>

/* LLM input variant 2: small-diverse */

int main()
{
    /* ---------- S‑box (plain int array, no const) ---------- */
    int sbox[256] = {
        255,254,253,252,251,250,249,248,247,246,245,244,243,242,241,240,
        239,238,237,236,235,234,233,232,231,230,229,228,227,226,225,224,
        223,222,221,220,219,218,217,216,215,214,213,212,211,210,209,208,
        207,206,205,204,203,202,201,200,199,198,197,196,195,194,193,192,
        191,190,189,188,187,186,185,184,183,182,181,180,179,178,177,176,
        175,174,173,172,171,170,169,168,167,166,165,164,163,162,161,160,
        159,158,157,156,155,154,153,152,151,150,149,148,147,146,145,144,
        143,142,141,140,139,138,137,136,135,134,133,132,131,130,129,128,
        127,126,125,124,123,122,121,120,119,118,117,116,115,114,113,112,
        111,110,109,108,107,106,105,104,103,102,101,100,99,98,97,96,
        95,94,93,92,91,90,89,88,87,86,85,84,83,82,81,80,
        79,78,77,76,75,74,73,72,71,70,69,68,67,66,65,64,
        63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
        47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    };

    /* ---------- Rcon (plain int array) ---------- */
    int rcon[15] = {
        0x9A,0x4D,0xAB,0xD8,0x6C,0x36,0x1B,0x80,
        0x40,0x20,0x10,0x08,0x04,0x02,0x01
    };

    /* ---------- Helper lambda for xtime (GF(2^8) multiply by 2) ---------- */
    auto xtime = [&](int x)->int{
        return ((x<<1) ^ ((x>>7)&1)*0x1b) & 0xFF;
    };

    /* ---------- Fixed 256‑bit key (32 bytes) ---------- */
    int key[32] = {
        0x00,0x01,0x02,0x03,
        0x04,0x05,0x06,0x07,
        0x08,0x09,0x0A,0x0B,
        0x0C,0x0D,0x0E,0x0F,
        0x10,0x11,0x12,0x13,
        0x14,0x15,0x16,0x17,
        0x18,0x19,0x1A,0x1B,
        0x1C,0x1D,0x1E,0x1F
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

    /* ---------- Plaintext (one block) ---------- */
    int pt[16] = {
        0x00,0x01,0x02,0x03,
        0x04,0x05,0x06,0x07,
        0x08,0x09,0x0A,0x0B,
        0x0C,0x0D,0x0E,0x0F
    };

    /* ---------- Nonce (single block, used as OCB nonce) ---------- */
    int nonce[16] = {
        0xF0,0xF1,0xF2,0xF3,
        0xF4,0xF5,0xF6,0xF7,
        0xF8,0xF9,0xFA,0xFB,
        0xFC,0xFD,0xFE,0xFF
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

        /* Row 1 left 1 */
        t = s[1];  s[1]=s[5];  s[5]=s[9];  s[9]=s[13]; s[13]=t;

        /* Row 2 left 2 */
        t = s[2];  s[2]=s[10]; s[10]=t;
        t = s[6];  s[6]=s[14]; s[14]=t;

        /* Row 3 left 3 (right 1) */
        t = s[3];  s[3]=s[15]; s[15]=s[11]; s[11]=s[7]; s[7]=t;
    };

    /* ---------- Helper lambda: MixColumns (manual, reordered ops) ---------- */
    auto mixColumns = [&](int* s){
        int a0,a1,a2,a3;
        /* Column 0 */
        a0 = s[0]; a1 = s[4]; a2 = s[8]; a3 = s[12];
        s[0] = xtime(a0) ^ (xtime(a1) ^ a1) ^ a2 ^ a3;
        s[4] = a0 ^ xtime(a1) ^ (xtime(a2) ^ a2) ^ a3;
        s[8] = a0 ^ a1 ^ xtime(a2) ^ (xtime(a3) ^ a3);
        s[12]= (xtime(a0) ^ a0) ^ a1 ^ a2 ^ xtime(a3);
        /* Column 1 */
        a0 = s[1]; a1 = s[5]; a2 = s[9]; a3 = s[13];
        s[1] = xtime(a0) ^ (xtime(a1) ^ a1) ^ a2 ^ a3;
        s[5] = a0 ^ xtime(a1) ^ (xtime(a2) ^ a2) ^ a3;
        s[9] = a0 ^ a1 ^ xtime(a2) ^ (xtime(a3) ^ a3);
        s[13]= (xtime(a0) ^ a0) ^ a1 ^ a2 ^ xtime(a3);
        /* Column 2 */
        a0 = s[2]; a1 = s[6]; a2 = s[10]; a3 = s[14];
        s[2] = xtime(a0) ^ (xtime(a1) ^ a1) ^ a2 ^ a3;
        s[6] = a0 ^ xtime(a1) ^ (xtime(a2) ^ a2) ^ a3;
        s[10]= a0 ^ a1 ^ xtime(a2) ^ (xtime(a3) ^ a3);
        s[14]= (xtime(a0) ^ a0) ^ a1 ^ a2 ^ xtime(a3);
        /* Column 3 */
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

        /* Initial round */
        addRoundKey(state,0);

        /* 13 full rounds (manual unroll) */
        int round = 1;
        while(round <= 13){
            subBytes(state);
            shiftRows(state);
            mixColumns(state);
            addRoundKey(state,round);
            round = round + 1;
        }

        /* Final round (no MixColumns) */
        subBytes(state);
        shiftRows(state);
        addRoundKey(state,14);

        j = 0;
        while(j < 16){ out[j] = state[j]; j = j + 1; }
    };

    /* ---------- Compute L = AES(Zero block) (used for OCB) ---------- */
    int zero[16];
    int k = 0;
    while(k < 16){ zero[k] = 0; k = k + 1; }
    int L[16];
    aesEncryptBlock(zero, L);

    /* ---------- OCB offset for first block (here simply L) ---------- */
    int offset[16];
    k = 0;
    while(k < 16){ offset[k] = L[k]; k = k + 1; }

    /* ---------- Ciphertext = AES( PT xor offset ) xor offset ---------- */
    int tmp[16];
    k = 0;
    while(k < 16){ tmp[k] = pt[k] ^ offset[k]; k = k + 1; }
    int enc[16];
    aesEncryptBlock(tmp, enc);
    int ct[16];
    k = 0;
    while(k < 16){ ct[k] = enc[k] ^ offset[k]; k = k + 1; }

    /* ---------- Checksum (single block) ---------- */
    int checksum[16];
    k = 0;
    while(k < 16){ checksum[k] = pt[k]; k = k + 1; }

    /* ---------- Tag = AES( offset xor checksum ) xor offset ---------- */
    int tagIn[16];
    k = 0;
    while(k < 16){ tagIn[k] = offset[k] ^ checksum[k]; k = k + 1; }
    int tagEnc[16];
    aesEncryptBlock(tagIn, tagEnc);
    int tag[16];
    k = 0;
    while(k < 16){ tag[k] = tagEnc[k] ^ offset[k]; k = k + 1; }

    /* ---------- Output ciphertext and tag ---------- */
    std::cout << "Ciphertext: ";
    k = 0;
    while(k < 16){
        std::cout << std::hex;
        if(ct[k] < 16) std::cout << '0';
        std::cout << ct[k];
        if(k != 15) std::cout << ' ';
        k = k + 1;
    }
    std::cout << std::dec << std::endl;

    std::cout << "Tag:        ";
    k = 0;
    while(k < 16){
        std::cout << std::hex;
        if(tag[k] < 16) std::cout << '0';
        std::cout << tag[k];
        if(k != 15) std::cout << ' ';
        k = k + 1;
    }
    std::cout << std::dec << std::endl;

    /* ---------- Clean up ---------- */
    delete [] w;

    return 0;
}
