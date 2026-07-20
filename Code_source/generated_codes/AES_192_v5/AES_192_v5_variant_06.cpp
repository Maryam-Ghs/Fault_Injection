// AES‑192 implementation – version #5
//  - only int / float types
//  - uses std::vector for all buffers
//  - manual loop unrolling and expanded multi‑step operations
//  - input: internally generated deterministic ordered data
//  - prints ciphertexts as hex
/* LLM input variant 6: ordered-structured */

#include <iostream>
#include <vector>
#include <iomanip>
#include <cstdlib>
#include <ctime>

// -------------------------------------------------------------------
//  S‑box (static, non‑const as required)
static int sbox[256] = {
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

// -------------------------------------------------------------------
//  Multiplication by x (xtime) in GF(2^8)
int xtime(int v) {
    int r = v << 1;
    if (v & 0x80) r ^= 0x1b;
    return r & 0xff;
}

// -------------------------------------------------------------------
//  Key schedule for AES‑192 (Nb=4, Nk=6, Nr=12)
void expandKey(const std::vector<int>& key, std::vector<int>& schedule) {
    // schedule will hold 52 words -> 52*4 = 208 bytes
    int i = 0;
    // first Nk words are the key itself
    while (i < 24) {
        schedule[i] = key[i];
        ++i;
    }

    // Rcon values (only first 8 needed for AES‑192)
    int rcon[8] = {1,2,4,8,16,32,64,128};

    int temp[4];
    while (i < 208) {
        // copy previous word
        temp[0] = schedule[i-4];
        temp[1] = schedule[i-3];
        temp[2] = schedule[i-2];
        temp[3] = schedule[i-1];

        if ((i/4) % 6 == 0) {               // every Nk words
            // RotWord
            int t = temp[0];
            temp[0] = temp[1];
            temp[1] = temp[2];
            temp[2] = temp[3];
            temp[3] = t;
            // SubWord
            temp[0] = sbox[temp[0]];
            temp[1] = sbox[temp[1]];
            temp[2] = sbox[temp[2]];
            temp[3] = sbox[temp[3]];
            // Rcon
            temp[0] ^= rcon[(i/24)-1];
        } else if ((i/4) % 6 == 4) {
            // SubWord only
            temp[0] = sbox[temp[0]];
            temp[1] = sbox[temp[1]];
            temp[2] = sbox[temp[2]];
            temp[3] = sbox[temp[3]];
        }

        // XOR with word Nk positions back
        schedule[i]   = schedule[i-24] ^ temp[0];
        schedule[i+1] = schedule[i-23] ^ temp[1];
        schedule[i+2] = schedule[i-22] ^ temp[2];
        schedule[i+3] = schedule[i-21] ^ temp[3];
        i += 4;
    }
}

// -------------------------------------------------------------------
//  SubBytes (manual unrolling)
void subBytes(std::vector<int>& state) {
    state[0]  = sbox[state[0]];   state[1]  = sbox[state[1]];
    state[2]  = sbox[state[2]];   state[3]  = sbox[state[3]];
    state[4]  = sbox[state[4]];   state[5]  = sbox[state[5]];
    state[6]  = sbox[state[6]];   state[7]  = sbox[state[7]];
    state[8]  = sbox[state[8]];   state[9]  = sbox[state[9]];
    state[10] = sbox[state[10]];  state[11] = sbox[state[11]];
    state[12] = sbox[state[12]];  state[13] = sbox[state[13]];
    state[14] = sbox[state[14]];  state[15] = sbox[state[15]];
}

// -------------------------------------------------------------------
//  ShiftRows (manual unrolling)
void shiftRows(std::vector<int>& st) {
    // row 1 shift left by 1
    int t = st[1];
    st[1]  = st[5];
    st[5]  = st[9];
    st[9]  = st[13];
    st[13] = t;

    // row 2 shift left by 2
    t = st[2];
    int u = st[6];
    st[2] = st[10];
    st[6] = st[14];
    st[10] = t;
    st[14] = u;

    // row 3 shift left by 3 (right by 1)
    t = st[15];
    st[15] = st[11];
    st[11] = st[7];
    st[7]  = st[3];
    st[3]  = t;
}

// -------------------------------------------------------------------
//  MixColumns (manual unrolling, expanded steps)
void mixColumns(std::vector<int>& st) {
    for (int c = 0; c < 4; ++c) {
        int i0 = c*4;
        int i1 = i0 + 1;
        int i2 = i0 + 2;
        int i3 = i0 + 3;

        int a0 = st[i0];
        int a1 = st[i1];
        int a2 = st[i2];
        int a3 = st[i3];

        int t0 = xtime(a0) ^ (xtime(a1) ^ a1) ^ a2 ^ a3;
        int t1 = a0 ^ xtime(a1) ^ (xtime(a2) ^ a2) ^ a3;
        int t2 = a0 ^ a1 ^ xtime(a2) ^ (xtime(a3) ^ a3);
        int t3 = (xtime(a0) ^ a0) ^ a1 ^ a2 ^ xtime(a3);

        st[i0] = t0;
        st[i1] = t1;
        st[i2] = t2;
        st[i3] = t3;
    }
}

// -------------------------------------------------------------------
//  AddRoundKey (manual unrolling)
void addRoundKey(std::vector<int>& st, const std::vector<int>& sch, int round) {
    int base = round * 16;
    st[0]  ^= sch[base];
    st[1]  ^= sch[base+1];
    st[2]  ^= sch[base+2];
    st[3]  ^= sch[base+3];
    st[4]  ^= sch[base+4];
    st[5]  ^= sch[base+5];
    st[6]  ^= sch[base+6];
    st[7]  ^= sch[base+7];
    st[8]  ^= sch[base+8];
    st[9]  ^= sch[base+9];
    st[10] ^= sch[base+10];
    st[11] ^= sch[base+11];
    st[12] ^= sch[base+12];
    st[13] ^= sch[base+13];
    st[14] ^= sch[base+14];
    st[15] ^= sch[base+15];
}

// -------------------------------------------------------------------
//  Encrypt a single 16‑byte block with AES‑192
void encryptBlock(std::vector<int>& block, const std::vector<int>& sch) {
    // initial round
    addRoundKey(block, sch, 0);

    // 11 full rounds
    for (int r = 1; r < 12; ++r) {
        subBytes(block);
        shiftRows(block);
        if (r != 12) mixColumns(block);
        addRoundKey(block, sch, r);
    }
}

// -------------------------------------------------------------------
//  Helper to print a block as hex
void printHex(const std::vector<int>& blk) {
    for (int i = 0; i < 16; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (blk[i] & 0xff);
        if (i != 15) std::cout << ' ';
    }
    std::cout << std::dec << '\n';
}

// -------------------------------------------------------------------
//  Main – deterministic ordered key & plaintext, encrypt, print
int main() {
    // ---- deterministic ordered 192‑bit key (24 bytes)
    std::vector<int> key(24);
    for (int i = 0; i < 24; ++i) {
        key[i] = (i * 13) % 256; // simple arithmetic progression
    }

    // ---- expand key
    std::vector<int> schedule(208);
    expandKey(key, schedule);

    // ---- deterministic ordered plaintext blocks (8 blocks)
    const int BLOCK_COUNT = 8;
    std::vector< std::vector<int> > plains(BLOCK_COUNT, std::vector<int>(16));
    for (int b = 0; b < BLOCK_COUNT; ++b) {
        for (int i = 0; i < 16; ++i) {
            plains[b][i] = (b * 16 + i) % 256; // sequential values across blocks
        }
    }

    // ---- encrypt each block
    std::cout << "Ciphertexts (hex):\n";
    for (int b = 0; b < BLOCK_COUNT; ++b) {
        std::vector<int> work = plains[b]; // copy
        encryptBlock(work, schedule);
        printHex(work);
    }

    return 0;
}
