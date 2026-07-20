#include <iostream>

/* LLM input variant 8: sparse-skewed */

int main() {
    // ---------- Tables (non‑const, as required) ----------
    int IP[64] = {
        58,50,42,34,26,18,10,2,60,52,44,36,28,20,12,4,
        62,54,46,38,30,22,14,6,64,56,48,40,32,24,16,8,
        57,49,41,33,25,17,9,1,59,51,43,35,27,19,11,3,
        61,53,45,37,29,21,13,5,63,55,47,39,31,23,15,7
    };
    int FP[64] = {
        40,8,48,16,56,24,64,32,39,7,47,15,55,23,63,31,
        38,6,46,14,54,22,62,30,37,5,45,13,53,21,61,29,
        36,4,44,12,52,20,60,28,35,3,43,11,51,19,59,27,
        34,2,42,10,50,18,58,26,33,1,41,9,49,17,57,25
    };
    int PC1[56] = {
        57,49,41,33,25,17,9,1,58,50,42,34,26,18,
        10,2,59,51,43,35,27,19,11,3,60,52,44,36,
        63,55,47,39,31,23,15,7,62,54,46,38,30,22,
        14,6,61,53,45,37,29,21,13,5,28,20,12,4
    };
    int PC2[48] = {
        14,17,11,24,1,5,3,28,15,6,21,10,
        23,19,12,4,26,8,16,7,27,20,13,2,
        41,52,31,37,47,55,30,40,51,45,33,48,
        44,49,39,56,34,53,46,42,50,36,29,32
    };
    int LS[16] = {1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1};
    int E[48] = {
        32,1,2,3,4,5,4,5,6,7,8,9,
        8,9,10,11,12,13,12,13,14,15,16,17,
        16,17,18,19,20,21,20,21,22,23,24,25,
        24,25,26,27,28,29,28,29,30,31,32,1
    };
    int P[32] = {
        16,7,20,21,29,12,28,17,
        1,15,23,26,5,18,31,10,
        2,8,24,14,32,27,3,9,
        19,13,30,6,22,11,4,25
    };
    int S[8][64] = {
        {14,4,13,1,2,15,11,8,3,10,6,12,5,9,0,7,
         0,15,7,4,14,2,13,1,10,6,12,11,9,5,3,8,
         4,1,14,8,13,6,2,11,15,12,9,7,3,10,5,0,
         15,12,8,2,4,9,1,7,5,11,3,14,10,0,6,13},
        {15,1,8,14,6,11,3,4,9,7,2,13,12,0,5,10,
         3,13,4,7,15,2,8,14,12,0,1,10,6,9,11,5,
         0,14,7,11,10,4,13,1,5,8,12,6,9,3,2,15,
         13,8,10,1,3,15,4,2,11,6,7,12,0,5,14,9},
        {10,0,9,14,6,3,15,5,1,13,12,7,11,4,2,8,
         13,7,0,9,3,4,6,10,2,8,5,14,12,11,15,1,
         13,6,4,9,8,15,3,0,11,1,2,12,5,10,14,7,
         1,10,13,0,6,9,8,7,4,15,14,3,11,5,2,12},
        {7,13,14,3,0,6,9,10,1,2,8,5,11,12,4,15,
         13,8,11,5,6,15,0,3,4,7,2,12,1,10,14,9,
         10,6,9,0,12,11,7,13,15,1,3,14,5,2,8,4,
         3,15,0,6,10,1,13,8,9,4,5,11,12,7,2,14},
        {2,12,4,1,7,10,11,6,8,5,3,15,13,0,14,9,
         14,11,2,12,4,7,13,1,5,0,15,10,3,9,8,6,
         4,2,1,11,10,13,7,8,15,9,12,5,6,3,0,14,
         11,8,12,7,1,14,2,13,6,15,0,9,10,4,5,3},
        {12,1,10,15,9,2,6,8,0,13,3,4,14,7,5,11,
         10,15,4,2,7,12,9,5,6,1,13,14,0,11,3,8,
         9,14,15,5,2,8,12,3,7,0,4,10,1,13,11,6,
         4,3,2,12,9,5,15,10,11,14,1,7,6,0,8,13},
        {4,11,2,14,15,0,8,13,3,12,9,7,5,10,6,1,
         13,0,11,7,4,9,1,10,14,3,5,12,2,15,8,6,
         1,4,11,13,12,3,7,14,10,15,6,8,0,5,9,2,
         6,11,13,8,1,4,10,7,9,5,0,15,14,2,3,12},
        {13,2,8,4,6,15,11,1,10,9,3,14,5,0,12,7,
         1,15,13,8,10,3,7,4,12,5,6,11,0,14,9,2,
         7,11,4,1,9,12,14,2,0,6,10,13,15,3,5,8,
         2,1,14,7,4,10,8,13,15,12,9,0,3,5,6,11}
    };

    // ---------- Test vectors ----------
    int pt[16] = {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0xFF,0x00,0xFF,0xFF
    };
    int key1[8] = {0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    int key2[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01};
    int key3[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    int iv[8]  = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

    // ---------- Helper lambdas (still inside main) ----------
    auto bytes_to_bits = [&](int *src, int *dst){
        int i,j;
        for(i=0;i<8;i++){
            for(j=0;j<8;j++){
                dst[i*8+j] = (src[i] >> (7-j)) & 1;
            }
        }
    };
    auto bits_to_bytes = [&](int *src, int *dst){
        int i,j;
        for(i=0;i<8;i++){
            int val=0;
            for(j=0;j<8;j++){
                val = (val<<1) | (src[i*8+j] & 1);
            }
            dst[i]=val;
        }
    };

    // ---------- Key schedule for each key ----------
    int sub1[16][48], sub2[16][48], sub3[16][48];
    int key_buf[56];
    int cd[56];
    int perm_buf[48];
    int i,j,k,round;

    // Function to generate subkeys for a given 8‑byte key
    auto gen_subkeys = [&](int *key, int sub[16][48]){
        int keybits[64];
        int pc1bits[56];
        int c[28], d[28];
        int tmpc[28], tmpd[28];
        int pc2bits[48];
        // key -> bits
        bytes_to_bits(key, keybits);
        // PC‑1
        for(i=0;i<56;i++) pc1bits[i] = keybits[PC1[i]-1];
        // split
        for(i=0;i<28;i++){ c[i]=pc1bits[i]; d[i]=pc1bits[28+i]; }
        // 16 rounds
        for(round=0; round<16; ++round){
            // left shift
            int shift = LS[round];
            for(i=0;i<28;i++){
                tmpc[i] = c[(i+shift)%28];
                tmpd[i] = d[(i+shift)%28];
            }
            for(i=0;i<28;i++){ c[i]=tmpc[i]; d[i]=tmpd[i]; }
            // combine C and D
            for(i=0;i<28;i++) cd[i]=c[i];
            for(i=0;i<28;i++) cd[28+i]=d[i];
            // PC‑2
            for(i=0;i<48;i++) pc2bits[i]=cd[PC2[i]-1];
            for(i=0;i<48;i++) sub[round][i]=pc2bits[i];
        }
    };

    gen_subkeys(key1, sub1);
    gen_subkeys(key2, sub2);
    gen_subkeys(key3, sub3);

    // ---------- OFB processing ----------
    int cur[8];
    for(i=0;i<8;i++) cur[i]=iv[i];
    int ct[16];
    int block_idx;
    for(block_idx=0; block_idx<2; ++block_idx){
        // ----- Triple‑DES encryption of 'cur' to produce keystream -----
        int inbits[64], outbits[64];
        int L[32], R[32], tmpL[32], tmpR[32];
        int expanded[48], xored[48], s_out[32];
        int round_key[48];
        // ---------- First DES (encrypt with K1) ----------
        bytes_to_bits(cur, inbits);
        // IP
        for(i=0;i<64;i++) outbits[i]=inbits[IP[i]-1];
        // split
        for(i=0;i<32;i++){ L[i]=outbits[i]; R[i]=outbits[32+i]; }
        // 16 rounds forward
        for(round=0; round<16; ++round){
            // expand R
            for(i=0;i<48;i++) expanded[i]=R[E[i]-1];
            // xor with subkey1
            for(i=0;i<48;i++) xored[i]=expanded[i]^sub1[round][i];
            // S‑box substitution
            for(i=0;i<8;i++){
                int row = (xored[i*6]<<1) | xored[i*6+5];
                int col = (xored[i*6+1]<<3) | (xored[i*6+2]<<2) |
                          (xored[i*6+3]<<1) | xored[i*6+4];
                int s_val = S[i][row*16+col];
                s_out[i*4+0] = (s_val>>3)&1;
                s_out[i*4+1] = (s_val>>2)&1;
                s_out[i*4+2] = (s_val>>1)&1;
                s_out[i*4+3] = s_val&1;
            }
            // P permutation
            for(i=0;i<32;i++) tmpR[i]=s_out[P[i]-1];
            // L xor F
            for(i=0;i<32;i++) tmpL[i]=L[i]^tmpR[i];
            // swap
            for(i=0;i<32;i++) L[i]=R[i];
            for(i=0;i<32;i++) R[i]=tmpL[i];
        }
        // combine and FP
        for(i=0;i<32;i++) outbits[i]=R[i];
        for(i=0;i<32;i++) outbits[32+i]=L[i];
        for(i=0;i<64;i++) inbits[i]=outbits[FP[i]-1];
        // ---------- Second DES (decrypt with K2) ----------
        // IP again
        for(i=0;i<64;i++) outbits[i]=inbits[IP[i]-1];
        for(i=0;i<32;i++){ L[i]=outbits[i]; R[i]=outbits[32+i]; }
        // 16 rounds reverse
        for(round=15; round>=0; --round){
            // expand R
            for(i=0;i<48;i++) expanded[i]=R[E[i]-1];
            // xor with subkey2 (reverse order)
            for(i=0;i<48;i++) xored[i]=expanded[i]^sub2[round][i];
            // S‑box
            for(i=0;i<8;i++){
                int row = (xored[i*6]<<1) | xored[i*6+5];
                int col = (xored[i*6+1]<<3) | (xored[i*6+2]<<2) |
                          (xored[i*6+3]<<1) | xored[i*6+4];
                int s_val = S[i][row*16+col];
                s_out[i*4+0] = (s_val>>3)&1;
                s_out[i*4+1] = (s_val>>2)&1;
                s_out[i*4+2] = (s_val>>1)&1;
                s_out[i*4+3] = s_val&1;
            }
            // P
            for(i=0;i<32;i++) tmpR[i]=s_out[P[i]-1];
            // L xor F
            for(i=0;i<32;i++) tmpL[i]=L[i]^tmpR[i];
            // swap
            for(i=0;i<32;i++) L[i]=R[i];
            for(i=0;i<32;i++) R[i]=tmpL[i];
        }
        // combine and FP
        for(i=0;i<32;i++) outbits[i]=R[i];
        for(i=0;i<32;i++) outbits[32+i]=L[i];
        for(i=0;i<64;i++) inbits[i]=outbits[FP[i]-1];
        // ---------- Third DES (encrypt with K3) ----------
        // IP again
        for(i=0;i<64;i++) outbits[i]=inbits[IP[i]-1];
        for(i=0;i<32;i++){ L[i]=outbits[i]; R[i]=outbits[32+i]; }
        // 16 rounds forward with K3
        for(round=0; round<16; ++round){
            // expand R
            for(i=0;i<48;i++) expanded[i]=R[E[i]-1];
            // xor with subkey3
            for(i=0;i<48;i++) xored[i]=expanded[i]^sub3[round][i];
            // S‑box
            for(i=0;i<8;i++){
                int row = (xored[i*6]<<1) | xored[i*6+5];
                int col = (xored[i*6+1]<<3) | (xored[i*6+2]<<2) |
                          (xored[i*6+3]<<1) | xored[i*6+4];
                int s_val = S[i][row*16+col];
                s_out[i*4+0] = (s_val>>3)&1;
                s_out[i*4+1] = (s_val>>2)&1;
                s_out[i*4+2] = (s_val>>1)&1;
                s_out[i*4+3] = s_val&1;
            }
            // P
            for(i=0;i<32;i++) tmpR[i]=s_out[P[i]-1];
            // L xor F
            for(i=0;i<32;i++) tmpL[i]=L[i]^tmpR[i];
            // swap
            for(i=0;i<32;i++) L[i]=R[i];
            for(i=0;i<32;i++) R[i]=tmpL[i];
        }
        // combine and FP to get keystream block
        for(i=0;i<32;i++) outbits[i]=R[i];
        for(i=0;i<32;i++) outbits[32+i]=L[i];
        for(i=0;i<64;i++) inbits[i]=outbits[FP[i]-1];
        int ks[8];
        bits_to_bytes(inbits, ks);

        // ----- XOR with plaintext to produce ciphertext -----
        for(i=0;i<8;i++){
            ct[block_idx*8 + i] = pt[block_idx*8 + i] ^ ks[i];
        }

        // ----- Update 'cur' for next OFB block (use ks as next input) -----
        for(i=0;i<8;i++) cur[i]=ks[i];
    }

    // ---------- Output ----------
    std::cout << "Ciphertext (hex): ";
    for(i=0;i<16;i++){
        int hi = (ct[i] >> 4) & 0xF;
        int lo = ct[i] & 0xF;
        char h1 = (hi<10)?('0'+hi):('A'+hi-10);
        char h2 = (lo<10)?('0'+lo):('A'+lo-10);
        std::cout << h1 << h2;
        if(i!=15) std::cout << " ";
    }
    std::cout << std::endl;
    return 0;
}
