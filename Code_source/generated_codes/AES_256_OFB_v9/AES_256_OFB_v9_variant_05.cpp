/******************************************************************
 * AES‑256‑OFB implementation – version #9
 *  * only int / float types (no unsigned, double, long, const)
 *  * all data on the heap (new / delete)
 *  * manual loop unrolling and multi‑step arithmetic
 *  * everything lives inside main()
 ******************************************************************/

/* LLM input variant 5: duplicate-heavy */

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <ctime>

int main()
{
    /*--------------------------------------------------------------
     * 1.  Parameters and deterministic data generation
     *-------------------------------------------------------------*/
    int blockSize = 16;                 // AES block = 16 bytes
    int keyBytes  = 32;                 // 256‑bit key
    int rounds    = 14;                 // AES‑256 has 14 rounds
    int plainLen  = 64;                 // example length (multiple of 16)

    int *key      = new int[keyBytes];
    int *iv       = new int[blockSize];
    int *plain    = new int[plainLen];
    int *cipher   = new int[plainLen];
    int *stream   = new int[blockSize]; // keystream block for OFB

    // Fill key with a repeated value (0x7F)
    for (int i = 0; i < keyBytes; ++i)   key[i] = 0x7F;
    // Fill IV with the same repeated value (0x7F)
    for (int i = 0; i < blockSize; ++i) iv[i]  = 0x7F;
    // Fill plaintext with a repeated pattern (0x55)
    for (int i = 0; i < plainLen; ++i)  plain[i] = 0x55;

    /*--------------------------------------------------------------
     * 2.  S‑box and Rcon (stored as signed int arrays)
     *-------------------------------------------------------------*/
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

    int rcon[15] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36,
                    0x6C,0xD8,0xAB,0x4D,0x9A};

    /*--------------------------------------------------------------
     * 3.  Key expansion – produce 15 round keys (16 bytes each)
     *-------------------------------------------------------------*/
    int *w = new int[60*4];                     // 60 words × 4 bytes
    // copy the original key (8 words)
    for (int i = 0; i < 8; ++i)
    {
        int base = i*4;
        w[base+0] = key[base+0];
        w[base+1] = key[base+1];
        w[base+2] = key[base+2];
        w[base+3] = key[base+3];
    }

    // helper lambda: xtime (multiply by 2 in GF(2^8))
    auto xtime = [&](int x)->int
    {
        int r = x << 1;
        if (x & 0x80) r ^= 0x1B;
        return r & 0xFF;
    };

    // generate remaining words
    for (int i = 8; i < 60; ++i)
    {
        int t0 = w[(i-1)*4+0];
        int t1 = w[(i-1)*4+1];
        int t2 = w[(i-1)*4+2];
        int t3 = w[(i-1)*4+3];

        if (i % 8 == 0)                 // RotWord + SubWord + Rcon
        {
            // RotWord
            int r0 = t1, r1 = t2, r2 = t3, r3 = t0;
            // SubWord
            r0 = sbox[r0]; r1 = sbox[r1]; r2 = sbox[r2]; r3 = sbox[r3];
            // Rcon
            r0 ^= rcon[i/8];
            t0 = r0; t1 = r1; t2 = r2; t3 = r3;
        }
        else if (i % 8 == 4)            // SubWord only
        {
            t0 = sbox[t0]; t1 = sbox[t1]; t2 = sbox[t2]; t3 = sbox[t3];
        }

        int src = (i-8)*4;
        w[i*4+0] = w[src+0] ^ t0;
        w[i*4+1] = w[src+1] ^ t1;
        w[i*4+2] = w[src+2] ^ t2;
        w[i*4+3] = w[src+3] ^ t3;
    }

    // pack round keys (15 × 16 bytes)
    int *roundKey = new int[(rounds+1)*blockSize];
    for (int r = 0; r <= rounds; ++r)
    {
        for (int c = 0; c < 4; ++c)          // 4 words per round
        {
            int src = (r*4 + c)*4;
            int dst = r*blockSize + c*4;
            roundKey[dst+0] = w[src+0];
            roundKey[dst+1] = w[src+1];
            roundKey[dst+2] = w[src+2];
            roundKey[dst+3] = w[src+3];
        }
    }

    /*--------------------------------------------------------------
     * 4.  Block encryption lambda (AES‑256, fully unrolled)
     *-------------------------------------------------------------*/
    auto encryptBlock = [&](int *in, int *out)
    {
        int s0 = in[0];  int s1 = in[1];  int s2 = in[2];  int s3 = in[3];
        int s4 = in[4];  int s5 = in[5];  int s6 = in[6];  int s7 = in[7];
        int s8 = in[8];  int s9 = in[9];  int s10 = in[10];int s11 = in[11];
        int s12 = in[12];int s13 = in[13];int s14 = in[14];int s15 = in[15];

        // ---- AddRoundKey (round 0) ----
        int rk = 0;
        s0 ^= roundKey[rk+0];  s1 ^= roundKey[rk+1];
        s2 ^= roundKey[rk+2];  s3 ^= roundKey[rk+3];
        s4 ^= roundKey[rk+4];  s5 ^= roundKey[rk+5];
        s6 ^= roundKey[rk+6];  s7 ^= roundKey[rk+7];
        s8 ^= roundKey[rk+8];  s9 ^= roundKey[rk+9];
        s10^= roundKey[rk+10]; s11^= roundKey[rk+11];
        s12^= roundKey[rk+12]; s13^= roundKey[rk+13];
        s14^= roundKey[rk+14]; s15^= roundKey[rk+15];

        // ---- 13 full rounds (1 … 13) ----
        for (int round = 1; round < rounds; ++round)
        {
            // SubBytes (unrolled)
            s0 = sbox[s0];   s1 = sbox[s1];   s2 = sbox[s2];   s3 = sbox[s3];
            s4 = sbox[s4];   s5 = sbox[s5];   s6 = sbox[s6];   s7 = sbox[s7];
            s8 = sbox[s8];   s9 = sbox[s9];   s10= sbox[s10];  s11= sbox[s11];
            s12= sbox[s12];  s13= sbox[s13];  s14= sbox[s14];  s15= sbox[s15];

            // ShiftRows (unrolled, column‑major layout)
            // row 1 shift left 1
            int t1 = s1;  s1 = s5;  s5 = s9;  s9 = s13; s13 = t1;
            // row 2 shift left 2
            int t2 = s2;  int t6 = s6;  s2 = s10; s6 = s14; s10 = t2; s14 = t6;
            // row 3 shift left 3 (right 1)
            int t3 = s15; s15 = s11; s11 = s7; s7 = s3; s3 = t3;

            // MixColumns (unrolled, 4 columns)
            // column 0
            int a0=s0, a1=s4, a2=s8, a3=s12;
            int m0 = xtime(a0) ^ (xtime(a1) ^ a1) ^ a2 ^ a3;
            int m1 = a0 ^ xtime(a1) ^ (xtime(a2) ^ a2) ^ a3;
            int m2 = a0 ^ a1 ^ xtime(a2) ^ (xtime(a3) ^ a3);
            int m3 = (xtime(a0) ^ a0) ^ a1 ^ a2 ^ xtime(a3);
            s0=m0; s4=m1; s8=m2; s12=m3;
            // column 1
            a0=s1; a1=s5; a2=s9; a3=s13;
            m0 = xtime(a0) ^ (xtime(a1) ^ a1) ^ a2 ^ a3;
            m1 = a0 ^ xtime(a1) ^ (xtime(a2) ^ a2) ^ a3;
            m2 = a0 ^ a1 ^ xtime(a2) ^ (xtime(a3) ^ a3);
            m3 = (xtime(a0) ^ a0) ^ a1 ^ a2 ^ xtime(a3);
            s1=m0; s5=m1; s9=m2; s13=m3;
            // column 2
            a0=s2; a1=s6; a2=s10; a3=s14;
            m0 = xtime(a0) ^ (xtime(a1) ^ a1) ^ a2 ^ a3;
            m1 = a0 ^ xtime(a1) ^ (xtime(a2) ^ a2) ^ a3;
            m2 = a0 ^ a1 ^ xtime(a2) ^ (xtime(a3) ^ a3);
            m3 = (xtime(a0) ^ a0) ^ a1 ^ a2 ^ xtime(a3);
            s2=m0; s6=m1; s10=m2; s14=m3;
            // column 3
            a0=s3; a1=s7; a2=s11; a3=s15;
            m0 = xtime(a0) ^ (xtime(a1) ^ a1) ^ a2 ^ a3;
            m1 = a0 ^ xtime(a1) ^ (xtime(a2) ^ a2) ^ a3;
            m2 = a0 ^ a1 ^ xtime(a2) ^ (xtime(a3) ^ a3);
            m3 = (xtime(a0) ^ a0) ^ a1 ^ a2 ^ xtime(a3);
            s3=m0; s7=m1; s11=m2; s15=m3;

            // AddRoundKey
            rk = round*blockSize;
            s0 ^= roundKey[rk+0];  s1 ^= roundKey[rk+1];
            s2 ^= roundKey[rk+2];  s3 ^= roundKey[rk+3];
            s4 ^= roundKey[rk+4];  s5 ^= roundKey[rk+5];
            s6 ^= roundKey[rk+6];  s7 ^= roundKey[rk+7];
            s8 ^= roundKey[rk+8];  s9 ^= roundKey[rk+9];
            s10^= roundKey[rk+10]; s11^= roundKey[rk+11];
            s12^= roundKey[rk+12]; s13^= roundKey[rk+13];
            s14^= roundKey[rk+14]; s15^= roundKey[rk+15];
        }

        // ---- Final round (no MixColumns) ----
        // SubBytes
        s0 = sbox[s0];   s1 = sbox[s1];   s2 = sbox[s2];   s3 = sbox[s3];
        s4 = sbox[s4];   s5 = sbox[s5];   s6 = sbox[s6];   s7 = sbox[s7];
        s8 = sbox[s8];   s9 = sbox[s9];   s10= sbox[s10];  s11= sbox[s11];
        s12= sbox[s12];  s13= sbox[s13];  s14= sbox[s14];  s15= sbox[s15];

        // ShiftRows
        int t1 = s1;  s1 = s5;  s5 = s9;  s9 = s13; s13 = t1;
        int t2 = s2;  int t6 = s6;  s2 = s10; s6 = s14; s10 = t2; s14 = t6;
        int t3 = s15; s15 = s11; s11 = s7; s7 = s3; s3 = t3;

        // AddRoundKey (round 14)
        rk = rounds*blockSize;
        s0 ^= roundKey[rk+0];  s1 ^= roundKey[rk+1];
        s2 ^= roundKey[rk+2];  s3 ^= roundKey[rk+3];
        s4 ^= roundKey[rk+4];  s5 ^= roundKey[rk+5];
        s6 ^= roundKey[rk+6];  s7 ^= roundKey[rk+7];
        s8 ^= roundKey[rk+8];  s9 ^= roundKey[rk+9];
        s10^= roundKey[rk+10]; s11^= roundKey[rk+11];
        s12^= roundKey[rk+12]; s13^= roundKey[rk+13];
        s14^= roundKey[rk+14]; s15^= roundKey[rk+15];

        // write output
        out[0] = s0;  out[1] = s1;  out[2] = s2;  out[3] = s3;
        out[4] = s4;  out[5] = s5;  out[6] = s6;  out[7] = s7;
        out[8] = s8;  out[9] = s9;  out[10]= s10; out[11]= s11;
        out[12]= s12; out[13]= s13; out[14]= s14; out[15]= s15;
    };

    /*--------------------------------------------------------------
     * 5.  OFB mode encryption
     *-------------------------------------------------------------*/
    // start with IV as the first keystream block
    for (int i = 0; i < blockSize; ++i) stream[i] = iv[i];

    int blocks = plainLen / blockSize;
    for (int b = 0; b < blocks; ++b)
    {
        // encrypt current stream to obtain next keystream block
        int next[16];
        encryptBlock(stream, next);
        // XOR plaintext with keystream -> ciphertext
        for (int i = 0; i < blockSize; ++i)
        {
            int pt = plain[b*blockSize + i];
            int ct = pt ^ next[i];
            cipher[b*blockSize + i] = ct;
        }
        // OFB: new stream = keystream block
        for (int i = 0; i < blockSize; ++i) stream[i] = next[i];
    }

    /*--------------------------------------------------------------
     * 6.  Output (hexadecimal)
     *-------------------------------------------------------------*/
    std::cout << "Plaintext : ";
    for (int i = 0; i < plainLen; ++i)
        std::cout << std::hex << std::setw(2) << std::setfill('0') << plain[i];
    std::cout << std::dec << "\n";

    std::cout << "Ciphertext: ";
    for (int i = 0; i < plainLen; ++i)
        std::cout << std::hex << std::setw(2) << std::setfill('0') << cipher[i];
    std::cout << std::dec << "\n";

    /*--------------------------------------------------------------
     * 7.  Cleanup
     *-------------------------------------------------------------*/
    delete[] key;
    delete[] iv;
    delete[] plain;
    delete[] cipher;
    delete[] stream;
    delete[] w;
    delete[] roundKey;

    return 0;
}
