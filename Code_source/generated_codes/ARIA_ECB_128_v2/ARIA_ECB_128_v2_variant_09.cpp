#include <iostream>
#include <iomanip>

int main()
{
    /* -------------------------------------------------------------
       ARIA-128 ECB encryption – one block
       Everything is placed on the stack, only int is used.
       ------------------------------------------------------------- */

    /* LLM input variant 9: medium-deterministic-random */
    /* ----------- test vector (deterministic) -------------------- */
    int pt[16] = {
        0x3a,0x7c,0x1f,0x4b,0x92,0xd5,0x68,0xe1,
        0x0c,0xb3,0x5a,0x27,0x9e,0x44,0xf2,0x81
    };
    int mk[16] = {
        0x6d,0x02,0xa9,0xf4,0x33,0x5e,0xc8,0x11,
        0x7b,0x9a,0x0d,0xe6,0x58,0x20,0xb7,0x3c
    };

    /* ----------- S‑boxes (identical to AES) --------------------- */
    int sb1[256] = {
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
    int sb2[256] = {
        82,9,106,213,48,54,165,56,191,4,199,2,104,113,156,255,
        61,54,54,204,229,71,46,124,254,111,127,56,70,43,174,233,
        111,2,231,87,0,229,248,135,101,38,100,69,5,104,247,8,
        106,239,27,9,0,45,169,221,110,91,4,112,54,62,0,158,
        207,33,173,104,252,20,41,205,147,27,78,2,113,245,33,194,
        122,116,92,91,180,107,24,55,226,238,71,44,232,197,196,226,
        99,89,5,70,232,115,106,166,70,150,169,221,144,6,250,225,
        230,176,226,23,71,228,65,136,180,242,215,182,238,251,65,166,
        209,194,100,225,180,155,84,2,113,33,248,34,185,143,30,73,
        247,115,127,91,84,139,74,75,106,103,73,117,143,124,225,40,
        22,69,165,215,191,41,63,87,100,0,61,73,150,88,6,2,
        117,239,8,40,174,84,74,203,150,145,90,30,240,4,225,254,
        180,60,0,91,127,5,16,145,236,0,44,42,96,20,47,0,
        24,0,33,146,98,2,166,0,50,0,117,70,86,129,252,167,
        0,1,194,89,119,168,20,103,0,93,69,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    };

    /* ----------- Galois Field multiplication (int only) -------- */
    auto xtime = [](int a)->int{
        int res = a << 1;
        if (a & 0x80) res ^= 0x11b;
        return res & 0xff;
    };
    auto mul2 = [&](int a)->int{ return xtime(a); };
    auto mul3 = [&](int a)->int{ return mul2(a) ^ a; };
    auto mul1 = [&](int a)->int{ return a; };

    /* ----------- linear diffusion (matrix) ---------------------- */
    auto diffuse = [&](int *src, int *dst)
    {
        int i = 0;
        while (i < 4)
        {
            int a0 = src[i];
            int a1 = src[i+4];
            int a2 = src[i+8];
            int a3 = src[i+12];

            int b0 = mul2(a0) ^ mul1(a1) ^ mul1(a2) ^ mul3(a3);
            int b1 = mul3(a0) ^ mul2(a1) ^ mul1(a2) ^ mul1(a3);
            int b2 = mul1(a0) ^ mul3(a1) ^ mul2(a2) ^ mul1(a3);
            int b3 = mul1(a0) ^ mul1(a1) ^ mul3(a2) ^ mul2(a3);

            dst[i]   = b0;
            dst[i+4] = b1;
            dst[i+8] = b2;
            dst[i+12]= b3;
            i = i + 1;
        }
    };

    /* ----------- Substitution ----------------------------------- */
    auto subst = [&](int *src, int *dst, int use_sb2)
    {
        int idx = 0;
        while (idx < 16)
        {
            if (use_sb2) dst[idx] = sb2[src[idx]];
            else        dst[idx] = sb1[src[idx]];
            idx = idx + 1;
        }
    };

    /* ----------- round constant (64‑bit) ------------------------ */
    int rc[6][8] = {
        {0x51,0x7c,0xc1,0xb7,0x27,0x22,0x0a,0x94},
        {0x35,0x28,0xc5,0xc6,0xcd,0x5e,0x98,0x5a},
        {0x0a,0x8b,0x9c,0x7d,0x8d,0x9c,0x6a,0x4b},
        {0x5c,0x3a,0xb6,0xb2,0xe0,0xd0,0xf0,0xe1},
        {0x3b,0x5f,0x5c,0x8a,0x1c,0x2d,0x3e,0x4f},
        {0x2a,0x1b,0x0c,0x0d,0x9e,0x8f,0x7a,0x6b}
    };

    /* ----------- key schedule (13 round keys) ------------------- */
    int rkey[13][16];

    int wk[8][16];
    int i = 0;
    while (i < 16) { wk[0][i] = mk[i]; ++i; }
    i = 0;
    while (i < 16) { wk[1][i] = mk[i]; ++i; }
    i = 0;
    while (i < 16) { wk[2][i] = mk[i]; ++i; }
    i = 0;
    while (i < 16) { wk[3][i] = mk[i]; ++i; }

    int tmp[16];
    int tmp2[16];

    int round = 0;
    while (round < 6)
    {
        i = 0;
        while (i < 16) { tmp[i] = wk[1][i] ^ wk[2][i]; ++i; }
        i = 0;
        while (i < 8)  { tmp[i] ^= rc[round][i]; ++i; }
        subst(tmp, tmp2, 0);
        diffuse(tmp2, tmp);
        i = 0;
        while (i < 16) { wk[4][i] = wk[0][i] ^ tmp[i]; ++i; }

        i = 0;
        while (i < 16) { tmp[i] = wk[2][i] ^ wk[3][i]; ++i; }
        i = 0;
        while (i < 8)  { tmp[i] ^= rc[round][i]; ++i; }
        subst(tmp, tmp2, 1);
        diffuse(tmp, tmp2);
        i = 0;
        while (i < 16) { wk[5][i] = wk[1][i] ^ tmp[i]; ++i; }

        i = 0;
        while (i < 16) { tmp[i] = wk[3][i] ^ wk[4][i]; ++i; }
        i = 0;
        while (i < 8)  { tmp[i] ^= rc[round][i]; ++i; }
        subst(tmp, tmp2, 0);
        diffuse(tmp2, tmp);
        i = 0;
        while (i < 16) { wk[6][i] = wk[2][i] ^ tmp[i]; ++i; }

        i = 0;
        while (i < 16) { tmp[i] = wk[4][i] ^ wk[5][i]; ++i; }
        i = 0;
        while (i < 8)  { tmp[i] ^= rc[round][i]; ++i; }
        subst(tmp, tmp2, 1);
        diffuse(tmp2, tmp);
        i = 0;
        while (i < 16) { wk[7][i] = wk[3][i] ^ tmp[i]; ++i; }

        i = 0;
        while (i < 16) { wk[0][i] = wk[4][i]; ++i; }
        i = 0;
        while (i < 16) { wk[1][i] = wk[5][i]; ++i; }
        i = 0;
        while (i < 16) { wk[2][i] = wk[6][i]; ++i; }
        i = 0;
        while (i < 16) { wk[3][i] = wk[7][i]; ++i; }

        ++round;
    }

    i = 0;
    while (i < 16) { rkey[0][i] = wk[0][i]; ++i; }
    i = 0;
    while (i < 16) { rkey[1][i] = wk[1][i]; ++i; }
    i = 0;
    while (i < 16) { rkey[2][i] = wk[2][i]; ++i; }
    i = 0;
    while (i < 16) { rkey[3][i] = wk[3][i]; ++i; }
    i = 0;
    while (i < 16) { rkey[4][i] = wk[4][i]; ++i; }
    i = 0;
    while (i < 16) { rkey[5][i] = wk[5][i]; ++i; }
    i = 0;
    while (i < 16) { rkey[6][i] = wk[6][i]; ++i; }
    i = 0;
    while (i < 16) { rkey[7][i] = wk[7][i]; ++i; }

    i = 0;
    while (i < 16) { rkey[8][i]  = rkey[0][i] ^ rkey[1][i]; ++i; }
    i = 0;
    while (i < 16) { rkey[9][i]  = rkey[2][i] ^ rkey[3][i]; ++i; }
    i = 0;
    while (i < 16) { rkey[10][i] = rkey[4][i] ^ rkey[5][i]; ++i; }
    i = 0;
    while (i < 16) { rkey[11][i] = rkey[6][i] ^ rkey[7][i]; ++i; }
    i = 0;
    while (i < 16) { rkey[12][i] = rkey[8][i] ^ rkey[9][i]; ++i; }

    int state[16];
    i = 0;
    while (i < 16) { state[i] = pt[i] ^ rkey[0][i]; ++i; }

    int round_idx = 1;
    while (round_idx <= 11)
    {
        int sb_sel = (round_idx % 2);
        subst(state, tmp, sb_sel);
        diffuse(tmp, state);
        i = 0;
        while (i < 16) { state[i] ^= rkey[round_idx][i]; ++i; }
        ++round_idx;
    }

    int final_sb = (round_idx % 2);
    subst(state, tmp, final_sb);
    i = 0;
    while (i < 16) { state[i] = tmp[i] ^ rkey[12][i]; ++i; }

    std::cout << "Plaintext : ";
    i = 0;
    while (i < 16) { std::cout << std::hex << std::setw(2) << std::setfill('0') << pt[i]; ++i; }
    std::cout << std::dec << "\n";

    std::cout << "Key       : ";
    i = 0;
    while (i < 16) { std::cout << std::hex << std::setw(2) << std::setfill('0') << mk[i]; ++i; }
    std::cout << std::dec << "\n";

    std::cout << "Cipher    : ";
    i = 0;
    while (i < 16) { std::cout << std::hex << std::setw(2) << std::setfill('0') << state[i]; ++i; }
    std::cout << std::dec << "\n";

    return 0;
}
