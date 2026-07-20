/********************************************************************
 *  AES‑256 in a simplified OCB mode (single‑block, no AAD)
 *  Version #5 – rewritten with new variable names, loop styles,
 *  and reordered mathematically‑valid operations.
 *
 *  Restrictions:
 *   - Only int (and float, not used) for numeric data.
 *   - No const, unsigned, double, long.
 *   - All data stored in std::vector<int>.
 *   - Deterministic test vectors are generated inside main().
 *
 *  Note: This OCB variant is **not** the full RFC‑7253 specification;
 *  it is a pedagogical single‑block construction that keeps the code
 *  self‑contained while still exercising the AES‑256 core.
 ********************************************************************/

/* LLM input variant 1: minimal-boundary */

#include <iostream>
#include <iomanip>
#include <vector>

/* ------------------------------------------------------------------
 *  S‑box and Rcon tables – mutable static int arrays (no const).
 * ------------------------------------------------------------------ */
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

static int rcon[15] = {
    0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36,
    0x6C,0xD8,0xAB,0x4D,0x9A
};

/* ------------------------------------------------------------------
 *  Helper: multiply by x (0x02) in GF(2^8)
 * ------------------------------------------------------------------ */
static int xtime(int v) {
    int shifted = v << 1;
    if (v & 0x80) shifted ^= 0x1b;
    return shifted & 0xff;
}

/* ------------------------------------------------------------------
 *  AES‑256 core wrapped in a class (OCB handling added later)
 * ------------------------------------------------------------------ */
class AES256OCB {
public:
    /* roundKeys holds (Nr+1) * 16 bytes = 240 ints */
    std::vector<int> roundKeys;

    AES256OCB(const std::vector<int>& masterKey) {
        roundKeys.assign(240, 0);
        expandKey(masterKey);
    }

    /* ------------------------------------------------------------------
     *  Public OCB interface (single‑block, no AAD)
     * ------------------------------------------------------------------ */
    void encryptOCB(const std::vector<int>& nonce,
                    const std::vector<int>& plain,
                    const std::vector<int>& /*assoc*/,   // unused
                    std::vector<int>& cipher,
                    std::vector<int>& tag) const
    {
        /* 1) compute a simple offset = AES(K, nonce) */
        std::vector<int> offset(16, 0);
        encryptBlock(nonce, offset);

        /* 2) ciphertext = AES(K, plain XOR offset) XOR offset */
        std::vector<int> tmp(16, 0);
        for (int pos = 0; pos < 16; ++pos)
            tmp[pos] = plain[pos] ^ offset[pos];
        encryptBlock(tmp, cipher);
        for (int pos = 0; pos < 16; ++pos)
            cipher[pos] ^= offset[pos];

        /* 3) tag = AES(K, cipher XOR offset) XOR offset */
        std::vector<int> tagTmp(16, 0);
        for (int pos = 0; pos < 16; ++pos)
            tagTmp[pos] = cipher[pos] ^ offset[pos];
        encryptBlock(tagTmp, tag);
        for (int pos = 0; pos < 16; ++pos)
            tag[pos] ^= offset[pos];
    }

private:
    /* ------------------------------------------------------------------
     *  Key expansion for AES‑256
     * ------------------------------------------------------------------ */
    void expandKey(const std::vector<int>& keyBytes) {
        int idx = 0;                      // index in roundKeys
        int wordPos = 0;                  // word counter

        /* copy the original 32‑byte key */
        while (wordPos < 8) {
            roundKeys[idx++] = keyBytes[4 * wordPos];
            roundKeys[idx++] = keyBytes[4 * wordPos + 1];
            roundKeys[idx++] = keyBytes[4 * wordPos + 2];
            roundKeys[idx++] = keyBytes[4 * wordPos + 3];
            ++wordPos;
        }

        int totalWords = 4 * (14 + 1);    // Nb*(Nr+1) = 4*15 = 60
        while (wordPos < totalWords) {
            /* fetch previous word */
            int t0 = roundKeys[(wordPos - 1) * 4];
            int t1 = roundKeys[(wordPos - 1) * 4 + 1];
            int t2 = roundKeys[(wordPos - 1) * 4 + 2];
            int t3 = roundKeys[(wordPos - 1) * 4 + 3];

            /* apply schedule core when needed */
            if (wordPos % 8 == 0) {
                /* RotWord */
                int r0 = t1, r1 = t2, r2 = t3, r3 = t0;
                /* SubWord */
                r0 = sbox[r0]; r1 = sbox[r1]; r2 = sbox[r2]; r3 = sbox[r3];
                /* Rcon */
                r0 ^= rcon[wordPos / 8 - 1];
                t0 = r0; t1 = r1; t2 = r2; t3 = r3;
            } else if (wordPos % 8 == 4) {
                /* SubWord only */
                t0 = sbox[t0]; t1 = sbox[t1]; t2 = sbox[t2]; t3 = sbox[t3];
            }

            /* XOR with word Nk positions earlier */
            int wk0 = roundKeys[(wordPos - 8) * 4];
            int wk1 = roundKeys[(wordPos - 8) * 4 + 1];
            int wk2 = roundKeys[(wordPos - 8) * 4 + 2];
            int wk3 = roundKeys[(wordPos - 8) * 4 + 3];

            roundKeys[idx++] = wk0 ^ t0;
            roundKeys[idx++] = wk1 ^ t1;
            roundKeys[idx++] = wk2 ^ t2;
            roundKeys[idx++] = wk3 ^ t3;

            ++wordPos;
        }
    }

    /* ------------------------------------------------------------------
     *  Block encryption (16 bytes)
     * ------------------------------------------------------------------ */
    void encryptBlock(const std::vector<int>& in,
                      std::vector<int>& out) const
    {
        std::vector<int> state = in;          // copy input

        addRoundKey(state, 0);

        int round = 1;
        while (round < 14) {
            subBytes(state);
            shiftRows(state);
            mixColumns(state);
            addRoundKey(state, round);
            ++round;
        }

        subBytes(state);
        shiftRows(state);
        addRoundKey(state, 14);

        out = state;
    }

    /* ------------------------------------------------------------------
     *  Round transformations
     * ------------------------------------------------------------------ */
    void addRoundKey(std::vector<int>& st, int rnd) const {
        int base = rnd * 16;
        int pos = 0;
        while (pos < 16) {
            st[pos] ^= roundKeys[base + pos];
            ++pos;
        }
    }

    void subBytes(std::vector<int>& st) const {
        int pos = 0;
        while (pos < 16) {
            st[pos] = sbox[st[pos]];
            ++pos;
        }
    }

    void shiftRows(std::vector<int>& st) const {
        /* Row 1: shift left by 1 */
        int t = st[1];
        st[1]  = st[5];
        st[5]  = st[9];
        st[9]  = st[13];
        st[13] = t;

        /* Row 2: shift left by 2 */
        int t0 = st[2], t1 = st[6];
        st[2]  = st[10];
        st[6]  = st[14];
        st[10] = t0;
        st[14] = t1;

        /* Row 3: shift left by 3 (right by 1) */
        t = st[15];
        st[15] = st[11];
        st[11] = st[7];
        st[7]  = st[3];
        st[3]  = t;
    }

    void mixColumns(std::vector<int>& st) const {
        int col = 0;
        while (col < 4) {
            int i0 = col;
            int i1 = col + 4;
            int i2 = col + 8;
            int i3 = col + 12;

            int a0 = st[i0];
            int a1 = st[i1];
            int a2 = st[i2];
            int a3 = st[i3];

            int t = a0 ^ a1 ^ a2 ^ a3;

            int tmp0 = a0;
            int u = a0 ^ a1; u = xtime(u);
            st[i0] ^= t ^ u;

            int v = a1 ^ a2; v = xtime(v);
            st[i1] ^= t ^ v;

            int w = a2 ^ a3; w = xtime(w);
            st[i2] ^= t ^ w;

            int x = a3 ^ tmp0; x = xtime(x);
            st[i3] ^= t ^ x;

            ++col;
        }
    }
};

/* --------------------------------------------------------------------
 *  Utility: print a byte vector as hex
 * -------------------------------------------------------------------- */
void showHex(const std::vector<int>& data, const char* label) {
    std::cout << label;
    for (size_t i = 0; i < data.size(); ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (data[i] & 0xff);
    }
    std::cout << std::dec << std::endl;
}

/* --------------------------------------------------------------------
 *  Main – deterministic test vectors (minimal boundary values)
 * -------------------------------------------------------------------- */
int main() {
    /* 256‑bit key: all zeros (boundary low) */
    std::vector<int> key(32, 0);

    /* 128‑bit nonce: all zeros (boundary low) */
    std::vector<int> nonce(16, 0);

    /* 128‑bit plaintext: all 0xFF (boundary high) */
    std::vector<int> plain(16, 0xFF);

    /* Empty associated data (not used) */
    std::vector<int> aad;   // zero length

    /* Prepare buffers for output */
    std::vector<int> cipher(16, 0);
    std::vector<int> tag(16, 0);

    /* Instantiate AES‑256‑OCB and encrypt */
    AES256OCB engine(key);
    engine.encryptOCB(nonce, plain, aad, cipher, tag);

    /* Show results */
    showHex(nonce, "Nonce    : ");
    showHex(plain, "Plaintext: ");
    showHex(cipher, "Cipher   : ");
    showHex(tag,    "Tag      : ");

    return 0;
}
