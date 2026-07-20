/* LLM input variant 1: minimal-boundary */
// AES‑256‑CCM implementation – version #8
// All code lives inside main(), uses only int and float, no const/unsigned/double/long.

#include <cstdio>
#include <cstdlib>
#include <ctime>

int main() {
    // Deterministic minimal test data (no randomness)
    int key[32];
    for (int i = 0; i < 32; ++i) key[i] = i & 0xFF;   // 0,1,2,...,31

    int nonce[12];
    for (int i = 0; i < 12; ++i) nonce[i] = i & 0xFF; // 0,1,2,...,11

    int plain[16];
    for (int i = 0; i < 16; ++i) plain[i] = i & 0xFF;  // 0,1,2,...,15

    // ------------------------------------------------------------
    // 1.  S‑box and round constants (stack allocation, plain int[])
    // ------------------------------------------------------------
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
    int rcon[15] = { 0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36,
                    0x6C,0xD8,0xAB,0x4D,0x9A };

    // ------------------------------------------------------------
    // 3.  Round keys – 15 × 16‑byte blocks (stack array)
    //    Simplified schedule: each round key = previous key xor Rcon
    // ------------------------------------------------------------
    int roundKey[15][16];
    for (int i = 0; i < 16; ++i) roundKey[0][i] = key[i];
    for (int i = 0; i < 16; ++i) roundKey[1][i] = key[16+i];

    for (int r = 2; r < 15; ++r) {
        // temp = last 4 bytes of previous round key, rotated & subbed
        int t0 = sbox[ roundKey[r-1][13] ];
        int t1 = sbox[ roundKey[r-1][14] ];
        int t2 = sbox[ roundKey[r-1][15] ];
        int t3 = sbox[ roundKey[r-1][12] ];

        t0 = t0 ^ rcon[r-2];                 // add round constant to first byte

        // first 4 bytes of new round key
        roundKey[r][0] = roundKey[r-2][0] ^ t0;
        roundKey[r][1] = roundKey[r-2][1] ^ t1;
        roundKey[r][2] = roundKey[r-2][2] ^ t2;
        roundKey[r][3] = roundKey[r-2][3] ^ t3;

        // remaining bytes – XOR with 4‑byte word 2 rounds back
        roundKey[r][4]  = roundKey[r-2][4]  ^ roundKey[r][0];
        roundKey[r][5]  = roundKey[r-2][5]  ^ roundKey[r][1];
        roundKey[r][6]  = roundKey[r-2][6]  ^ roundKey[r][2];
        roundKey[r][7]  = roundKey[r-2][7]  ^ roundKey[r][3];
        roundKey[r][8]  = roundKey[r-2][8]  ^ roundKey[r][4];
        roundKey[r][9]  = roundKey[r-2][9]  ^ roundKey[r][5];
        roundKey[r][10] = roundKey[r-2][10] ^ roundKey[r][6];
        roundKey[r][11] = roundKey[r-2][11] ^ roundKey[r][7];
        roundKey[r][12] = roundKey[r-2][12] ^ roundKey[r][8];
        roundKey[r][13] = roundKey[r-2][13] ^ roundKey[r][9];
        roundKey[r][14] = roundKey[r-2][14] ^ roundKey[r][10];
        roundKey[r][15] = roundKey[r-2][15] ^ roundKey[r][11];
    }

    // ------------------------------------------------------------
    // 4.  Helper lambdas – all stay inside main()
    // ------------------------------------------------------------
    auto mul2 = [&](int x)->int{
        int shifted = (x << 1) & 0xFF;
        int reduced = ((x >> 7) & 1) * 0x1B;
        return (shifted ^ reduced) & 0xFF;
    };
    auto mul3 = [&](int x)->int{
        return (mul2(x) ^ x) & 0xFF;
    };

    // SubBytes – fully unrolled
    auto subBytes = [&](int s[16]){
        s[0] = sbox[s[0]];  s[1] = sbox[s[1]];  s[2] = sbox[s[2]];  s[3] = sbox[s[3]];
        s[4] = sbox[s[4]];  s[5] = sbox[s[5]];  s[6] = sbox[s[6]];  s[7] = sbox[s[7]];
        s[8] = sbox[s[8]];  s[9] = sbox[s[9]];  s[10]= sbox[s[10]]; s[11]= sbox[s[11]];
        s[12]= sbox[s[12]]; s[13]= sbox[s[13]]; s[14]= sbox[s[14]]; s[15]= sbox[s[15]];
    };

    // ShiftRows – unrolled
    auto shiftRows = [&](int s[16]){
        int t1 = s[1];  int t5 = s[5];  int t9 = s[9];  int t13 = s[13];
        s[1] = s[5];    s[5] = s[9];    s[9] = s[13];   s[13]= t1;

        int t2 = s[2];  int t6 = s[6];  int t10 = s[10]; int t14 = s[14];
        s[2] = s[10];   s[6] = s[14];   s[10]= t2;      s[14]= t6;

        int t3 = s[3];  int t7 = s[7];  int t11 = s[11]; int t15 = s[15];
        s[3] = s[15];   s[7] = t3;      s[11]= t7;      s[15]= t11;
    };

    // MixColumns – unrolled, column wise
    auto mixColumns = [&](int s[16]){
        int c0 = s[0], c1 = s[1], c2 = s[2], c3 = s[3];
        s[0] = mul2(c0) ^ mul3(c1) ^ c2 ^ c3;
        s[1] = c0 ^ mul2(c1) ^ mul3(c2) ^ c3;
        s[2] = c0 ^ c1 ^ mul2(c2) ^ mul3(c3);
        s[3] = mul3(c0) ^ c1 ^ c2 ^ mul2(c3);

        c0 = s[4]; c1 = s[5]; c2 = s[6]; c3 = s[7];
        s[4] = mul2(c0) ^ mul3(c1) ^ c2 ^ c3;
        s[5] = c0 ^ mul2(c1) ^ mul3(c2) ^ c3;
        s[6] = c0 ^ c1 ^ mul2(c2) ^ mul3(c3);
        s[7] = mul3(c0) ^ c1 ^ c2 ^ mul2(c3);

        c0 = s[8]; c1 = s[9]; c2 = s[10]; c3 = s[11];
        s[8] = mul2(c0) ^ mul3(c1) ^ c2 ^ c3;
        s[9] = c0 ^ mul2(c1) ^ mul3(c2) ^ c3;
        s[10]= c0 ^ c1 ^ mul2(c2) ^ mul3(c3);
        s[11]= mul3(c0) ^ c1 ^ c2 ^ mul2(c3);

        c0 = s[12]; c1 = s[13]; c2 = s[14]; c3 = s[15];
        s[12]= mul2(c0) ^ mul3(c1) ^ c2 ^ c3;
        s[13]= c0 ^ mul2(c1) ^ mul3(c2) ^ c3;
        s[14]= c0 ^ c1 ^ mul2(c2) ^ mul3(c3);
        s[15]= mul3(c0) ^ c1 ^ c2 ^ mul2(c3);
    };

    // AddRoundKey – unrolled XOR
    auto addRoundKey = [&](int s[16], int rk[16]){
        s[0] ^= rk[0];  s[1] ^= rk[1];  s[2] ^= rk[2];  s[3] ^= rk[3];
        s[4] ^= rk[4];  s[5] ^= rk[5];  s[6] ^= rk[6];  s[7] ^= rk[7];
        s[8] ^= rk[8];  s[9] ^= rk[9];  s[10]^= rk[10]; s[11]^= rk[11];
        s[12]^= rk[12]; s[13]^= rk[13]; s[14]^= rk[14]; s[15]^= rk[15];
    };

    // One AES‑256 block encryption (14 rounds + initial)
    auto aesEncrypt = [&](int in[16], int out[16]){
        int s[16];
        for (int i = 0; i < 16; ++i) s[i] = in[i];

        addRoundKey(s, roundKey[0]);

        for (int r = 1; r < 14; ++r) {
            subBytes(s);
            shiftRows(s);
            mixColumns(s);
            addRoundKey(s, roundKey[r]);
        }

        subBytes(s);
        shiftRows(s);
        addRoundKey(s, roundKey[14]);

        for (int i = 0; i < 16; ++i) out[i] = s[i];
    };

    // ------------------------------------------------------------
    // 5.  CCM – tag generation (CBC‑MAC) and encryption (CTR)
    // ------------------------------------------------------------
    // Flags for B0: M'=7 (16‑byte tag), L'=1 (2‑byte length field)
    int flags = 0x59;               // 0b01011001
    int b0[16];
    b0[0] = flags;
    for (int i = 0; i < 12; ++i) b0[1+i] = nonce[i];
    b0[13] = 0; b[14] = 0; b0[15] = 16;   // message length = 16 bytes

    // CBC‑MAC – start with zero block
    int mac[16] = {0};
    int block[16];
    // MAC over B0
    for (int i = 0; i < 16; ++i) block[i] = b0[i] ^ mac[i];
    aesEncrypt(block, mac);

    // MAC over plaintext (no associated data in this demo)
    for (int i = 0; i < 16; ++i) block[i] = plain[i] ^ mac[i];
    aesEncrypt(block, mac);

    // ------------------------------------------------------------
    // 6.  CTR encryption (nonce || counter)
    // ------------------------------------------------------------
    int ctr[16];
    ctr[0] = 0x01;                  // Flags for CTR (L'=1)
    for (int i = 0; i < 12; ++i) ctr[1+i] = nonce[i];
    ctr[13] = 0; ctr[14] = 0; ctr[15] = 1;   // counter = 1

    int keystream[16];
    aesEncrypt(ctr, keystream);

    int cipher[16];
    for (int i = 0; i < 16; ++i) cipher[i] = plain[i] ^ keystream[i];

    // ------------------------------------------------------------
    // 7.  Tag encryption (S0 = AES(K, nonce||0))
    // ------------------------------------------------------------
    ctr[15] = 0;               // counter = 0 for S0
    aesEncrypt(ctr, keystream);
    int tag[16];
    for (int i = 0; i < 16; ++i) tag[i] = mac[i] ^ keystream[i];

    // ------------------------------------------------------------
    // 8.  Output – hex dump (int printed as two‑digit hex)
    // ------------------------------------------------------------
    std::printf("Plaintext : ");
    for (int i = 0; i < 16; ++i) std::printf("%02X ", plain[i]);
    std::printf("\nCiphertext: ");
    for (int i = 0; i < 16; ++i) std::printf("%02X ", cipher[i]);
    std::printf("\nTag       : ");
    for (int i = 0; i < 16; ++i) std::printf("%02X ", tag[i]);
    std::printf("\n");

    return 0;
}
