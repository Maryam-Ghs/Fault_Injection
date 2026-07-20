#include <iostream>
#include <vector>
#include <iomanip>

/* LLM input variant 2: small-diverse */

// ------------------------------------------------------------
// Helper tables (no const, only int)
// ------------------------------------------------------------
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

static int rcon[11] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36,0x6C};

// ------------------------------------------------------------
// Basic byte operations
// ------------------------------------------------------------
int xtime(int b) {
    int r = b << 1;
    if (b & 0x80) r ^= 0x1b;
    return r & 0xFF;
}

// ------------------------------------------------------------
// AES core transformations (split into functions)
// ------------------------------------------------------------
void sub_bytes(std::vector<int>& st) {
    int i = 0;
    while (i < 16) {
        st[i] = sbox[st[i]];
        i = i + 1;
    }
}

void shift_rows(std::vector<int>& st) {
    // row 1 (index 1,5,9,13) rotate left by 1
    int t = st[1];
    st[1] = st[5];
    st[5] = st[9];
    st[9] = st[13];
    st[13] = t;
    // row 2 rotate left by 2
    int t0 = st[2];
    int t1 = st[6];
    st[2] = st[10];
    st[6] = st[14];
    st[10] = t0;
    st[14] = t1;
    // row 3 rotate left by 3 (right by 1)
    t = st[15];
    st[15] = st[11];
    st[11] = st[7];
    st[7] = st[3];
    st[3] = t;
}

void mix_columns(std::vector<int>& st) {
    int c = 0;
    while (c < 4) {
        int i0 = c * 4;
        int a0 = st[i0];
        int a1 = st[i0 + 1];
        int a2 = st[i0 + 2];
        int a3 = st[i0 + 3];

        int m0 = xtime(a0) ^ (xtime(a1) ^ a1) ^ a2 ^ a3;
        int m1 = a0 ^ xtime(a1) ^ (xtime(a2) ^ a2) ^ a3;
        int m2 = a0 ^ a1 ^ xtime(a2) ^ (xtime(a3) ^ a3);
        int m3 = (xtime(a0) ^ a0) ^ a1 ^ a2 ^ xtime(a3);

        st[i0]     = m0 & 0xFF;
        st[i0 + 1] = m1 & 0xFF;
        st[i0 + 2] = m2 & 0xFF;
        st[i0 + 3] = m3 & 0xFF;

        c = c + 1;
    }
}

void add_round_key(std::vector<int>& st, const std::vector<int>& rk, int offset) {
    int i = 0;
    while (i < 16) {
        st[i] = st[i] ^ rk[offset + i];
        i = i + 1;
    }
}

// ------------------------------------------------------------
// Key expansion for AES‑192 (13 round keys)
// ------------------------------------------------------------
std::vector<int> expand_key(const std::vector<int>& key) {
    std::vector<int> wk(208, 0);               // 13 * 16
    int i = 0;
    while (i < 24) {
        wk[i] = key[i];
        i = i + 1;
    }
    int bytes = 24;
    int r = 1;
    while (bytes < 208) {
        int t0 = wk[bytes - 4];
        int t1 = wk[bytes - 3];
        int t2 = wk[bytes - 2];
        int t3 = wk[bytes - 1];
        // RotWord
        int tmp = t0;
        t0 = t1; t1 = t2; t2 = t3; t3 = tmp;
        // SubWord
        t0 = sbox[t0]; t1 = sbox[t1]; t2 = sbox[t2]; t3 = sbox[t3];
        // Rcon
        t0 = t0 ^ rcon[r - 1];
        // first 4 bytes
        wk[bytes]     = wk[bytes - 24] ^ t0;
        wk[bytes + 1] = wk[bytes - 23] ^ t1;
        wk[bytes + 2] = wk[bytes - 22] ^ t2;
        wk[bytes + 3] = wk[bytes - 21] ^ t3;
        // next 8 bytes (no extra SubWord)
        int j = 4;
        while (j < 12) {
            wk[bytes + j] = wk[bytes + j - 4] ^ wk[bytes - 24 + j];
            j = j + 1;
        }
        bytes = bytes + 12;
        r = r + 1;
    }
    return wk;
}

// ------------------------------------------------------------
// AES‑192 block encryption (single 16‑byte block)
// ------------------------------------------------------------
void aes_encrypt_block(std::vector<int> block, const std::vector<int>& round_keys, std::vector<int>& out) {
    add_round_key(block, round_keys, 0);
    int round = 1;
    while (round < 12) {
        sub_bytes(block);
        shift_rows(block);
        mix_columns(block);
        add_round_key(block, round_keys, round * 16);
        round = round + 1;
    }
    sub_bytes(block);
    shift_rows(block);
    add_round_key(block, round_keys, 12 * 16);
    out = block;
}

// ------------------------------------------------------------
// GF(2^128) multiplication for GHASH
// ------------------------------------------------------------
std::vector<int> gf128_mul(const std::vector<int>& X, const std::vector<int>& Y) {
    std::vector<int> Z(16, 0);
    std::vector<int> V = Y;
    int i = 0;
    while (i < 128) {
        int byteIdx = i >> 3;
        int bitIdx  = 7 - (i & 7);
        int bit = (X[byteIdx] >> bitIdx) & 1;
        if (bit) {
            int k = 0;
            while (k < 16) {
                Z[k] ^= V[k];
                k = k + 1;
            }
        }
        // get LSB of V before shift
        int lsb = V[15] & 1;
        // shift V right by one bit
        int carry = 0;
        int j = 0;
        while (j < 16) {
            int newCarry = V[j] & 1;
            V[j] = (V[j] >> 1) | (carry << 7);
            carry = newCarry;
            j = j + 1;
        }
        if (lsb) {
            V[0] ^= 0xE1;   // R = 0xe1 00..00
        }
        i = i + 1;
    }
    return Z;
}

// ------------------------------------------------------------
// GHASH over AAD || Ciphertext || lengths
// ------------------------------------------------------------
std::vector<int> ghash(const std::vector<int>& H,
                       const std::vector<int>& aad,
                       const std::vector<int>& ct,
                       int aadBits,
                       int ctBits) {
    std::vector<int> Y(16, 0);
    // process AAD
    int pos = 0;
    while (pos < (int)aad.size()) {
        std::vector<int> block(16, 0);
        int k = 0;
        while (k < 16 && (pos + k) < (int)aad.size()) {
            block[k] = aad[pos + k];
            k = k + 1;
        }
        int m = 0;
        while (m < 16) {
            Y[m] ^= block[m];
            m = m + 1;
        }
        Y = gf128_mul(Y, H);
        pos = pos + 16;
    }
    // process ciphertext
    pos = 0;
    while (pos < (int)ct.size()) {
        std::vector<int> block(16, 0);
        int k = 0;
        while (k < 16 && (pos + k) < (int)ct.size()) {
            block[k] = ct[pos + k];
            k = k + 1;
        }
        int m = 0;
        while (m < 16) {
            Y[m] ^= block[m];
            m = m + 1;
        }
        Y = gf128_mul(Y, H);
        pos = pos + 16;
    }
    // length block
    std::vector<int> lenBlock(16, 0);
    // aad length (64‑bit big endian)
    for (int i = 0; i < 8; ++i) {
        lenBlock[7 - i] = (aadBits >> (i * 8)) & 0xFF;
    }
    // ciphertext length (64‑bit big endian)
    for (int i = 0; i < 8; ++i) {
        lenBlock[15 - i] = (ctBits >> (i * 8)) & 0xFF;
    }
    int m = 0;
    while (m < 16) {
        Y[m] ^= lenBlock[m];
        m = m + 1;
    }
    Y = gf128_mul(Y, H);
    return Y;
}

// ------------------------------------------------------------
// Increment 32‑bit counter (last 4 bytes)
// ------------------------------------------------------------
void inc32(std::vector<int>& ctr) {
    int i = 15;
    while (i >= 12) {
        ctr[i] = (ctr[i] + 1) & 0xFF;
        if (ctr[i] != 0) break;
        i = i - 1;
    }
}

// ------------------------------------------------------------
// GCM encryption (no AAD, single block)
// ------------------------------------------------------------
void gcm_encrypt(const std::vector<int>& key,
                 const std::vector<int>& nonce,
                 const std::vector<int>& pt,
                 std::vector<int>& ct,
                 std::vector<int>& tag) {

    // round keys
    std::vector<int> roundKeys = expand_key(key);

    // H = AES(K, 0^128)
    std::vector<int> zeroBlock(16, 0);
    std::vector<int> H(16);
    aes_encrypt_block(zeroBlock, roundKeys, H);

    // J0 = nonce || 0x00000001  (nonce length == 12)
    std::vector<int> J0 = nonce;
    J0.push_back(0);
    J0.push_back(0);
    J0.push_back(0);
    J0.push_back(1);

    // first counter
    std::vector<int> ctr = J0;
    inc32(ctr);

    // encrypt each block
    ct.clear();
    int pos = 0;
    while (pos < (int)pt.size()) {
        std::vector<int> enc(16);
        aes_encrypt_block(ctr, roundKeys, enc);
        std::vector<int> block(16, 0);
        int k = 0;
        while (k < 16 && (pos + k) < (int)pt.size()) {
            block[k] = pt[pos + k];
            k = k + 1;
        }
        int j = 0;
        while (j < 16) {
            block[j] ^= enc[j];
            j = j + 1;
        }
        int m = 0;
        while (m < 16 && (pos + m) < (int)pt.size()) {
            ct.push_back(block[m]);
            m = m + 1;
        }
        inc32(ctr);
        pos = pos + 16;
    }

    // GHASH
    std::vector<int> aad; // empty
    int aadBits = 0;
    int ctBits  = (int)ct.size() * 8;
    std::vector<int> S = ghash(H, aad, ct, aadBits, ctBits);

    // Tag = AES(K, J0) xor S
    std::vector<int> T(16);
    aes_encrypt_block(J0, roundKeys, T);
    int i = 0;
    while (i < 16) {
        tag.push_back(T[i] ^ S[i]);
        i = i + 1;
    }
}

// ------------------------------------------------------------
// Pretty‑print a byte vector as hex
// ------------------------------------------------------------
void print_hex(const std::vector<int>& buf, const char* label) {
    std::cout << label;
    for (size_t i = 0; i < buf.size(); ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (buf[i] & 0xFF);
    }
    std::cout << std::dec << std::endl;
}

// ------------------------------------------------------------
// Main – generate a tiny test vector and run GCM
// ------------------------------------------------------------
int main() {
    // 192‑bit key (24 bytes) – diverse values
    std::vector<int> key = {
        0x1f,0x2e,0x3d,0x4c,0x5b,0x6a,
        0x79,0x88,0x97,0xa6,0xb5,0xc4,
        0xd3,0xe2,0xf1,0x00,0x10,0x20,
        0x30,0x40,0x50,0x60,0x70,0x80
    };

    // 12‑byte nonce – varied pattern
    std::vector<int> nonce = {
        0x12,0x34,0x56,0x78,0x9a,0xbc,
        0xde,0xf0,0x11,0x22,0x33,0x02
    };

    // 16‑byte plaintext – mixed bytes
    std::vector<int> plaintext = {
        0xff,0x00,0xaa,0x55,0x12,0x34,0x56,0x78,
        0x9a,0xbc,0xcd,0xef,0x01,0x23,0x45,0x67
    };

    std::vector<int> ciphertext;
    std::vector<int> authTag;

    gcm_encrypt(key, nonce, plaintext, ciphertext, authTag);

    print_hex(plaintext, "Plaintext : ");
    print_hex(ciphertext, "Ciphertext: ");
    print_hex(authTag,   "Tag       : ");

    return 0;
}
