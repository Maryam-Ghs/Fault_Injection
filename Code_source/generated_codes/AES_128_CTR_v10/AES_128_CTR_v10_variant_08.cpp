/* LLM input variant 8: sparse-skewed */
// AES‑128‑CTR implementation – version #10
// ------------------------------------------------
// Restrictions applied:
//   * only int (no unsigned, long, double, const)
//   * std::vector for dynamic storage
//   * manual loop unrolling & expanded multi‑step arithmetic
//   * helper functions split the algorithm
// ------------------------------------------------

#include <iostream>
#include <vector>
#include <iomanip>

// ------------------------------------------------------------------
// 1.  Core tables (S‑box and Rcon) – plain int arrays, not const
// ------------------------------------------------------------------
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

static int rcon[10] = { 1,2,4,8,16,32,64,128,27,54 };

// ------------------------------------------------------------------
// 2.  Helper arithmetic (GF(2^8) multiplication)
// ------------------------------------------------------------------
int mul2(int x) {
    int r = x << 1;
    if (r & 0x100) r ^= 0x11b;
    return r & 0xff;
}
int mul3(int x) {
    return mul2(x) ^ x;
}

// ------------------------------------------------------------------
// 3.  Core AES transformations (unrolled loops)
// ------------------------------------------------------------------
void subBytes(std::vector<int>& st) {
    st[0] = sbox[st[0]];  st[1] = sbox[st[1]];  st[2] = sbox[st[2]];  st[3] = sbox[st[3]];
    st[4] = sbox[st[4]];  st[5] = sbox[st[5]];  st[6] = sbox[st[6]];  st[7] = sbox[st[7]];
    st[8] = sbox[st[8]];  st[9] = sbox[st[9]];  st[10] = sbox[st[10]]; st[11] = sbox[st[11]];
    st[12] = sbox[st[12]];st[13] = sbox[st[13]];st[14] = sbox[st[14]];st[15] = sbox[st[15]];
}

void shiftRows(std::vector<int>& st) {
    // row 1 (index 1,5,9,13) shift left by 1
    int t1 = st[1]; st[1] = st[5]; st[5] = st[9]; st[9] = st[13]; st[13] = t1;
    // row 2 (index 2,6,10,14) shift left by 2
    int t2 = st[2]; int t6 = st[6];
    st[2] = st[10]; st[6] = st[14]; st[10] = t2; st[14] = t6;
    // row 3 (index 3,7,11,15) shift left by 3 (right by 1)
    int t3 = st[15];
    st[15] = st[11]; st[11] = st[7]; st[7] = st[3]; st[3] = t3;
}

void mixColumns(std::vector<int>& st) {
    for (int c = 0; c < 4; ++c) {
        int i0 = 4*c, i1 = i0+1, i2 = i0+2, i3 = i0+3;
        int a0 = st[i0], a1 = st[i1], a2 = st[i2], a3 = st[i3];
        int r0 = mul2(a0) ^ mul3(a1) ^ a2 ^ a3;
        int r1 = a0 ^ mul2(a1) ^ mul3(a2) ^ a3;
        int r2 = a0 ^ a1 ^ mul2(a2) ^ mul3(a3);
        int r3 = mul3(a0) ^ a1 ^ a2 ^ mul2(a3);
        st[i0] = r0 & 0xff; st[i1] = r1 & 0xff; st[i2] = r2 & 0xff; st[i3] = r3 & 0xff;
    }
}

void addRoundKey(std::vector<int>& st, const std::vector<int>& ek, int off) {
    st[0] ^= ek[off];   st[1] ^= ek[off+1]; st[2] ^= ek[off+2]; st[3] ^= ek[off+3];
    st[4] ^= ek[off+4]; st[5] ^= ek[off+5]; st[6] ^= ek[off+6]; st[7] ^= ek[off+7];
    st[8] ^= ek[off+8]; st[9] ^= ek[off+9]; st[10] ^= ek[off+10]; st[11] ^= ek[off+11];
    st[12] ^= ek[off+12];st[13] ^= ek[off+13];st[14] ^= ek[off+14];st[15] ^= ek[off+15];
}

// ------------------------------------------------------------------
// 4.  Key expansion (produces 176‑byte expanded key)
// ------------------------------------------------------------------
void expandKey(const std::vector<int>& key, std::vector<int>& ek) {
    // copy original key
    for (int i = 0; i < 16; ++i) ek[i] = key[i];

    int bytesGenerated = 16;   // already have 16 bytes
    int rconIter = 0;

    while (bytesGenerated < 176) {
        // ----- temp = last 4 bytes -----
        int t0 = ek[bytesGenerated-4];
        int t1 = ek[bytesGenerated-3];
        int t2 = ek[bytesGenerated-2];
        int t3 = ek[bytesGenerated-1];

        // ----- every 16‑byte boundary: core transformation -----
        if ((bytesGenerated % 16) == 0) {
            // RotWord
            int tmp = t0; t0 = t1; t1 = t2; t2 = t3; t3 = tmp;
            // SubWord
            t0 = sbox[t0]; t1 = sbox[t1]; t2 = sbox[t2]; t3 = sbox[t3];
            // Rcon
            t0 ^= rcon[rconIter];
            ++rconIter;
        }

        // ----- XOR with word 4 positions earlier -----
        ek[bytesGenerated    ] = ek[bytesGenerated-16] ^ t0;
        ek[bytesGenerated + 1] = ek[bytesGenerated-15] ^ t1;
        ek[bytesGenerated + 2] = ek[bytesGenerated-14] ^ t2;
        ek[bytesGenerated + 3] = ek[bytesGenerated-13] ^ t3;

        bytesGenerated += 4;
    }
}

// ------------------------------------------------------------------
// 5.  Encrypt a single 16‑byte block
// ------------------------------------------------------------------
void encryptBlock(const std::vector<int>& inBlk,
                  const std::vector<int>& ek,
                  std::vector<int>& outBlk) {
    // copy input to state
    std::vector<int> st(16);
    for (int i = 0; i < 16; ++i) st[i] = inBlk[i];

    // initial AddRoundKey
    addRoundKey(st, ek, 0);

    // 9 main rounds
    for (int round = 1; round < 10; ++round) {
        subBytes(st);
        shiftRows(st);
        mixColumns(st);
        addRoundKey(st, ek, round*16);
    }

    // final round (no MixColumns)
    subBytes(st);
    shiftRows(st);
    addRoundKey(st, ek, 160);

    // output
    for (int i = 0; i < 16; ++i) outBlk[i] = st[i];
}

// ------------------------------------------------------------------
// 6.  Increment 128‑bit counter (little‑endian)
// ------------------------------------------------------------------
void incCounter(std::vector<int>& ctr) {
    for (int i = 15; i >= 0; --i) {
        ctr[i] = (ctr[i] + 1) & 0xff;
        if (ctr[i] != 0) break; // stop on no overflow
    }
}

// ------------------------------------------------------------------
// 7.  CTR mode encryption (plaintext length multiple of 16)
// ------------------------------------------------------------------
void ctrEncrypt(const std::vector<int>& plain,
                const std::vector<int>& key,
                std::vector<int>& cipher) {
    // key schedule
    std::vector<int> ek(176);
    expandKey(key, ek);

    // deterministic initial counter (nonce) – sparse pattern
    std::vector<int> counter(16, 0);
    counter[0] = 0x01;
    counter[15] = 0xFF;

    int blocks = (int)plain.size() / 16;
    cipher.resize(plain.size());

    for (int b = 0; b < blocks; ++b) {
        // encrypt counter to obtain keystream
        std::vector<int> keystream(16);
        encryptBlock(counter, ek, keystream);

        // XOR with plaintext
        for (int i = 0; i < 16; ++i) {
            int idx = b*16 + i;
            cipher[idx] = plain[idx] ^ keystream[i];
        }

        // next counter value
        incCounter(counter);
    }
}

// ------------------------------------------------------------------
// 8.  Main – deterministic sparse‑skewed data, run CTR, print hex tables
// ------------------------------------------------------------------
int main() {
    // ----- deterministic sparse 128‑bit key -----
    std::vector<int> key(16, 0);
    key[0] = 0x0A;   // only a few non‑zero bytes
    key[7] = 0x5C;
    key[15] = 0xFF;

    // ----- deterministic plaintext (6 blocks = 96 bytes) -----
    // Mostly zeros, with two dense clusters
    int blockCount = 6;
    std::vector<int> plaintext(blockCount * 16, 0);

    // Cluster 1: block 2 (indices 32‑39)
    for (int i = 0; i < 8; ++i) {
        plaintext[32 + i] = (i + 1) * 0x11;
    }

    // Cluster 2: block 5 (indices 80‑95) – alternating pattern
    for (int i = 0; i < 16; ++i) {
        plaintext[80 + i] = (i % 2 == 0) ? 0xAA : 0x55;
    }

    // ----- encrypt -----
    std::vector<int> ciphertext;
    ctrEncrypt(plaintext, key, ciphertext);

    // ----- display -----
    std::cout << "Key:\n";
    for (int i = 0; i < 16; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << key[i] << ' ';
        if ((i+1)%8==0) std::cout << '\n';
    }
    std::cout << "\nPlaintext:\n";
    for (size_t i = 0; i < plaintext.size(); ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << plaintext[i] << ' ';
        if ((i+1)%16==0) std::cout << '\n';
    }
    std::cout << "\nCiphertext (CTR):\n";
    for (size_t i = 0; i < ciphertext.size(); ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << ciphertext[i] << ' ';
        if ((i+1)%16==0) std::cout << '\n';
    }
    return 0;
}
