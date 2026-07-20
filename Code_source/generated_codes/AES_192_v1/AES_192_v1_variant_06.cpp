#include <bits/stdc++.h>
using namespace std;

/* LLM input variant 6: ordered-structured */

int main() {
    // ---------- S‑box ----------
    vector<int> sbox = {
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
    vector<int> rcon = {0,1,2,4,8,16,32,64,128,27,54,108};

    // ---------- multiplication tables ----------
    vector<int> mul2(256), mul3(256);
    for (int i = 0; i < 256; ++i) {
        int x = i << 1;
        mul2[i] = ((x & 0x100) ? (x ^ 0x11B) : x) & 0xFF;
        mul3[i] = (mul2[i] ^ i) & 0xFF;
    }

    // ---------- ordered‑structured inputs ----------
    vector<int> key192(24), plain(16);
    for (int i = 0; i < 24; ++i) key192[i] = i;   // sequential key bytes 0..23
    for (int i = 0; i < 16; ++i) plain[i] = i;    // sequential plaintext bytes 0..15

    // ---------- key expansion ----------
    const int Nk = 6, Nr = 12, Nb = 4;
    vector<int> roundKey((Nr+1)*16);
    // copy initial key
    for (int i = 0; i < 24; ++i) roundKey[i] = key192[i];
    // expand
    for (int i = Nk; i < Nb*(Nr+1); ++i) {
        int t0 = roundKey[(i-1)*4 + 0];
        int t1 = roundKey[(i-1)*4 + 1];
        int t2 = roundKey[(i-1)*4 + 2];
        int t3 = roundKey[(i-1)*4 + 3];

        int cond0 = (i % Nk == 0);
        int cond1 = (Nk > 6 && i % Nk == 4);
        // ----- RotWord + SubWord + Rcon when cond0 -----
        int r0 = t0, r1 = t1, r2 = t2, r3 = t3;
        // rotate
        r0 = t1; r1 = t2; r2 = t3; r3 = t0;
        // substitute
        r0 = sbox[r0]; r1 = sbox[r1]; r2 = sbox[r2]; r3 = sbox[r3];
        // add Rcon
        r0 ^= rcon[i/Nk];
        // apply conditionally
        t0 = cond0 ? r0 : t0;
        t1 = cond0 ? r1 : t1;
        t2 = cond0 ? r2 : t2;
        t3 = cond0 ? r3 : t3;
        // ----- SubWord only when cond1 -----
        t0 = cond1 ? sbox[t0] : t0;
        t1 = cond1 ? sbox[t1] : t1;
        t2 = cond1 ? sbox[t2] : t2;
        t3 = cond1 ? sbox[t3] : t3;

        // XOR with word Nk positions earlier
        int p0 = roundKey[(i-Nk)*4 + 0];
        int p1 = roundKey[(i-Nk)*4 + 1];
        int p2 = roundKey[(i-Nk)*4 + 2];
        int p3 = roundKey[(i-Nk)*4 + 3];
        roundKey[i*4 + 0] = (p0 ^ t0) & 0xFF;
        roundKey[i*4 + 1] = (p1 ^ t1) & 0xFF;
        roundKey[i*4 + 2] = (p2 ^ t2) & 0xFF;
        roundKey[i*4 + 3] = (p3 ^ t3) & 0xFF;
    }

    // ---------- encryption ----------
    vector<int> state = plain;
    // initial AddRoundKey
    for (int i = 0; i < 16; ++i) state[i] ^= roundKey[i];

    // main rounds
    for (int r = 1; r < Nr; ++r) {
        // SubBytes
        for (int i = 0; i < 16; ++i) state[i] = sbox[state[i]];
        // ShiftRows (in‑place using temp copy)
        vector<int> tmp = state;
        state[0]  = tmp[0];  state[4]  = tmp[4];  state[8]  = tmp[8];  state[12] = tmp[12];
        state[1]  = tmp[5];  state[5]  = tmp[9];  state[9]  = tmp[13]; state[13] = tmp[1];
        state[2]  = tmp[10]; state[6]  = tmp[14]; state[10] = tmp[2];  state[14] = tmp[6];
        state[3]  = tmp[15]; state[7]  = tmp[3];  state[11] = tmp[7];  state[15] = tmp[11];
        // MixColumns
        for (int c = 0; c < 4; ++c) {
            int a0 = state[c];
            int a1 = state[4 + c];
            int a2 = state[8 + c];
            int a3 = state[12 + c];
            int b0 = mul2[a0] ^ mul3[a1] ^ a2 ^ a3;
            int b1 = a0 ^ mul2[a1] ^ mul3[a2] ^ a3;
            int b2 = a0 ^ a1 ^ mul2[a2] ^ mul3[a3];
            int b3 = mul3[a0] ^ a1 ^ a2 ^ mul2[a3];
            state[c]      = b0 & 0xFF;
            state[4 + c]  = b1 & 0xFF;
            state[8 + c]  = b2 & 0xFF;
            state[12 + c] = b3 & 0xFF;
        }
        // AddRoundKey
        for (int i = 0; i < 16; ++i) state[i] ^= roundKey[r*16 + i];
    }

    // final round (no MixColumns)
    for (int i = 0; i < 16; ++i) state[i] = sbox[state[i]];
    // ShiftRows final
    {
        vector<int> tmp = state;
        state[0]  = tmp[0];  state[4]  = tmp[4];  state[8]  = tmp[8];  state[12] = tmp[12];
        state[1]  = tmp[5];  state[5]  = tmp[9];  state[9]  = tmp[13]; state[13] = tmp[1];
        state[2]  = tmp[10]; state[6]  = tmp[14]; state[10] = tmp[2];  state[14] = tmp[6];
        state[3]  = tmp[15]; state[7]  = tmp[3];  state[11] = tmp[7];  state[15] = tmp[11];
    }
    // final AddRoundKey
    for (int i = 0; i < 16; ++i) state[i] ^= roundKey[Nr*16 + i];

    // ---------- output ----------
    cout << "Ciphertext (hex): ";
    for (int i = 0; i < 16; ++i) {
        int v = state[i] & 0xFF;
        if (v < 16) cout << '0';
        cout << std::hex << v;
    }
    cout << endl;
    return 0;
}
