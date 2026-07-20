/* LLM input variant 2: small-diverse */
#include <iostream>
#include <vector>
#include <iomanip>

// ------------------------------------------------------------
// Simple AES‑128 implementation (only encryption)
// ------------------------------------------------------------
class AES128
{
public:
    // round keys = 11 * 16 bytes
    std::vector<int> rk;

    AES128(std::vector<int> key)   // key must be 16 bytes
    {
        keyExpansion(key);
    }

    // encrypt one 16‑byte block
    void encryptBlock(std::vector<int> in, std::vector<int> &out)
    {
        std::vector<int> state = in;          // 16 bytes

        addRoundKey(state, 0);

        int round;
        for (round = 1; round < 10; ++round)
        {
            subBytes(state);
            shiftRows(state);
            mixColumns(state);
            addRoundKey(state, round);
        }

        subBytes(state);
        shiftRows(state);
        addRoundKey(state, round);

        out = state;
    }

private:
    // ----------------------------------------------------------------
    // S‑box
    // ----------------------------------------------------------------
    static int sbox[256];

    // ----------------------------------------------------------------
    // Rijndael round constants
    // ----------------------------------------------------------------
    static int rcon[10];

    // ----------------------------------------------------------------
    // Key schedule
    // ----------------------------------------------------------------
    void keyExpansion(std::vector<int> key)
    {
        // first 16 bytes are the original key
        int i;
        for (i = 0; i < 16; ++i)
            rk.push_back(key[i]);

        int bytesGenerated = 16;
        int rconIter = 0;
        while (bytesGenerated < 176)
        {
            // take previous 4‑byte word
            int temp0 = rk[bytesGenerated - 4];
            int temp1 = rk[bytesGenerated - 3];
            int temp2 = rk[bytesGenerated - 2];
            int temp3 = rk[bytesGenerated - 1];

            // every 16‑byte block apply core
            if ((bytesGenerated % 16) == 0)
            {
                // rotate
                int t = temp0;
                temp0 = temp1;
                temp1 = temp2;
                temp2 = temp3;
                temp3 = t;

                // sub bytes
                temp0 = sbox[temp0];
                temp1 = sbox[temp1];
                temp2 = sbox[temp2];
                temp3 = sbox[temp3];

                // rcon
                temp0 ^= rcon[rconIter];
                ++rconIter;
            }

            // xor with word 16 bytes earlier
            int k0 = rk[bytesGenerated - 16] ^ temp0;
            int k1 = rk[bytesGenerated - 15] ^ temp1;
            int k2 = rk[bytesGenerated - 14] ^ temp2;
            int k3 = rk[bytesGenerated - 13] ^ temp3;

            rk.push_back(k0);
            rk.push_back(k1);
            rk.push_back(k2);
            rk.push_back(k3);
            bytesGenerated += 4;
        }
    }

    // ----------------------------------------------------------------
    // Helper transformations
    // ----------------------------------------------------------------
    void subBytes(std::vector<int> &state)
    {
        int i;
        for (i = 0; i < 16; ++i)
            state[i] = sbox[state[i]];
    }

    void shiftRows(std::vector<int> &state)
    {
        // row 1 shift left 1
        int t = state[1];
        state[1]  = state[5];
        state[5]  = state[9];
        state[9]  = state[13];
        state[13] = t;

        // row 2 shift left 2
        t = state[2];
        int u = state[6];
        state[2]  = state[10];
        state[6]  = state[14];
        state[10] = t;
        state[14] = u;

        // row 3 shift left 3 (right 1)
        t = state[3];
        state[3]  = state[15];
        state[15] = state[11];
        state[11] = state[7];
        state[7]  = t;
    }

    // multiplication by 2 in GF(2^8)
    int xtime(int x)
    {
        int hi = x & 0x80;
        x = (x << 1) & 0xFF;
        if (hi)
            x ^= 0x1B;
        return x;
    }

    void mixColumns(std::vector<int> &state)
    {
        int c;
        for (c = 0; c < 4; ++c)
        {
            int i0 = c * 4;
            int s0 = state[i0];
            int s1 = state[i0 + 1];
            int s2 = state[i0 + 2];
            int s3 = state[i0 + 3];

            int m0 = xtime(s0) ^ (xtime(s1) ^ s1) ^ s2 ^ s3;
            int m1 = s0 ^ xtime(s1) ^ (xtime(s2) ^ s2) ^ s3;
            int m2 = s0 ^ s1 ^ xtime(s2) ^ (xtime(s3) ^ s3);
            int m3 = (xtime(s0) ^ s0) ^ s1 ^ s2 ^ xtime(s3);

            state[i0]     = m0 & 0xFF;
            state[i0 + 1] = m1 & 0xFF;
            state[i0 + 2] = m2 & 0xFF;
            state[i0 + 3] = m3 & 0xFF;
        }
    }

    void addRoundKey(std::vector<int> &state, int round)
    {
        int i;
        for (i = 0; i < 16; ++i)
        {
            state[i] ^= rk[round * 16 + i];
        }
    }
};

// ----------------------------------------------------------------
// static tables
// ----------------------------------------------------------------
int AES128::sbox[256] = {
0x63,0x7c,0x77,0x7b,0xf2,0x6b,0x6f,0xc5,0x30,0x01,0x67,0x2b,0xfe,0xd7,0xab,0x76,
0xca,0x82,0xc9,0x7d,0xfa,0x59,0x47,0xf0,0xad,0xd4,0xa2,0xaf,0x9c,0xa4,0x72,0xc0,
0xb7,0xfd,0x93,0x26,0x36,0x3f,0xf7,0xcc,0x34,0xa5,0xe5,0xf1,0x71,0xd8,0x31,0x15,
0x04,0xc7,0x23,0xc3,0x18,0x96,0x05,0x9a,0x07,0x12,0x80,0xe2,0xeb,0x27,0xb2,0x75,
0x09,0x83,0x2c,0x1a,0x1b,0x6e,0x5a,0xa0,0x52,0x3b,0xd6,0xb3,0x29,0xe3,0x2f,0x84,
0x53,0xd1,0x00,0xed,0x20,0xfc,0xb1,0x5b,0x6a,0xcb,0xbe,0x39,0x4a,0x4c,0x58,0xcf,
0xd0,0xef,0xaa,0xfb,0x43,0x4d,0x33,0x85,0x45,0xf9,0x02,0x7f,0x50,0x3c,0x9f,0xa8,
0x51,0xa3,0x40,0x8f,0x92,0x9d,0x38,0xf5,0xbc,0xb6,0xda,0x21,0x10,0xff,0xf3,0xd2,
0xcd,0x0c,0x13,0xec,0x5f,0x97,0x44,0x17,0xc4,0xa7,0x7e,0x3d,0x64,0x5d,0x19,0x73,
0x60,0x81,0x4f,0xdc,0x22,0x2a,0x90,0x88,0x46,0xee,0xb8,0x14,0xde,0x5e,0x0b,0xdb,
0xe0,0x32,0x3a,0x0a,0x49,0x06,0x24,0x5c,0xc2,0xd3,0xac,0x62,0x91,0x95,0xe4,0x79,
0xe7,0xc8,0x37,0x6d,0x8d,0xd5,0x4e,0xa9,0x6c,0x56,0xf4,0xea,0x65,0x7a,0xae,0x08,
0xba,0x78,0x25,0x2e,0x1c,0xa6,0xb4,0xc6,0xe8,0xdd,0x74,0x1f,0x4b,0xbd,0x8b,0x8a,
0x70,0x3e,0xb5,0x66,0x48,0x03,0xf6,0x0e,0x61,0x35,0x57,0xb9,0x86,0xc1,0x1d,0x9e,
0xe1,0xf8,0x98,0x11,0x69,0xd9,0x8e,0x94,0x9b,0x1e,0x87,0xe9,0xce,0x55,0x28,0xdf,
0x8c,0xa1,0x89,0x0d,0xbf,0xe6,0x42,0x68,0x41,0x99,0x2d,0x0f,0xb0,0x54,0xbb,0x16};

int AES128::rcon[10] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1B,0x36};

// ----------------------------------------------------------------
// GF(2^128) doubling (used by OCB)
// ----------------------------------------------------------------
std::vector<int> dbl(const std::vector<int> &blk)
{
    std::vector<int> out(16);
    int carry = 0;
    int i;
    for (i = 15; i >= 0; --i)
    {
        int cur = blk[i];
        out[i] = ((cur << 1) & 0xFF) | carry;
        carry = (cur >> 7) & 1;
    }
    if (carry)
        out[15] ^= 0x87;
    return out;
}

// ----------------------------------------------------------------
// Count trailing zeros (used for offset selection)
// ----------------------------------------------------------------
int ntz(int x)
{
    int n = 0;
    while ((x & 1) == 0 && n < 32)
    {
        ++n;
        x >>= 1;
    }
    return n;
}

// ----------------------------------------------------------------
// OCB‑128 encryption (single‑pass, tag length 16)
// ----------------------------------------------------------------
class OCB128
{
public:
    AES128 aes;
    std::vector<int> L;          // L = AES(K, 0^128)
    std::vector<int> L_star;     // L* = dbl(L)
    std::vector<int> L_dollar;   // L$ = dbl(L*)

    OCB128(std::vector<int> key) : aes(key)
    {
        std::vector<int> zero(16, 0);
        aes.encryptBlock(zero, L);
        L_star   = dbl(L);
        L_dollar = dbl(L_star);
    }

    // encrypt plaintext with given nonce (12 bytes)
    void encrypt(std::vector<int> nonce,
                 std::vector<int> plain,
                 std::vector<int> &cipher,
                 std::vector<int> &tag)
    {
        // ------- offset_0 = AES(K, nonce || 0x00000001) ----------
        std::vector<int> nonceBlock(16, 0);
        int i;
        for (i = 0; i < 12; ++i)
            nonceBlock[i] = nonce[i];
        nonceBlock[15] = 0x01;                     // simple counter

        std::vector<int> offset;
        aes.encryptBlock(nonceBlock, offset);     // offset = Ktop

        // pre‑compute L_i up to needed size (we use up to 32)
        std::vector< std::vector<int> > L_i(33);
        L_i[0] = L_dollar;
        int j;
        for (j = 1; j <= 32; ++j)
            L_i[j] = dbl(L_i[j - 1]);

        std::vector<int> checksum(16, 0);
        std::vector<int> out;
        int blockCount = (int)plain.size() / 16;
        int leftover   = (int)plain.size() % 16;

        // -------- process full blocks ----------------------------
        int blkIdx;
        for (blkIdx = 1; blkIdx <= blockCount; ++blkIdx)
        {
            int tz = ntz(blkIdx);
            // offset = offset xor L_{tz}
            int k;
            for (k = 0; k < 16; ++k)
                offset[k] ^= L_i[tz][k];

            // get current plaintext block
            std::vector<int> m(16);
            for (k = 0; k < 16; ++k)
                m[k] = plain[(blkIdx - 1) * 16 + k];

            // ciphertext block = AES(K, m xor offset) xor offset
            std::vector<int> tmp(16);
            for (k = 0; k < 16; ++k)
                tmp[k] = m[k] ^ offset[k];

            std::vector<int> enc(16);
            aes.encryptBlock(tmp, enc);

            std::vector<int> c(16);
            for (k = 0; k < 16; ++k)
                c[k] = enc[k] ^ offset[k];

            // append to output
            for (k = 0; k < 16; ++k)
                out.push_back(c[k]);

            // checksum = checksum xor (m xor offset)
            for (k = 0; k < 16; ++k)
                checksum[k] ^= tmp[k];
        }

        // -------- handle final partial block ----------------------
        if (leftover > 0)
        {
            // offset = offset xor L*
            for (i = 0; i < 16; ++i)
                offset[i] ^= L_star[i];

            // pad = AES(K, offset)
            std::vector<int> pad(16);
            aes.encryptBlock(offset, pad);

            // ciphertext partial
            for (i = 0; i < leftover; ++i)
                out.push_back(plain[blockCount * 16 + i] ^ pad[i]);

            // checksum = checksum xor (partial || 0^{...})
            std::vector<int> tmp(16, 0);
            for (i = 0; i < leftover; ++i)
                tmp[i] = plain[blockCount * 16 + i];
            for (i = 0; i < 16; ++i)
                checksum[i] ^= tmp[i];
        }
        else
        {
            // no partial block – still need to adjust offset for tag
            // offset = offset xor L*
            for (i = 0; i < 16; ++i)
                offset[i] ^= L_star[i];
        }

        // -------- compute tag ------------------------------------
        // tag = AES(K, offset xor L$) xor checksum
        std::vector<int> tmpTagOff(16);
        for (i = 0; i < 16; ++i)
            tmpTagOff[i] = offset[i] ^ L_dollar[i];

        std::vector<int> tmpTag(16);
        aes.encryptBlock(tmpTagOff, tmpTag);

        for (i = 0; i < 16; ++i)
            tag.push_back(tmpTag[i] ^ checksum[i]);

        cipher = out;
    }
};

// ----------------------------------------------------------------
// Helper: print a byte vector as hex
// ----------------------------------------------------------------
void printHex(const std::vector<int> &v)
{
    int i;
    for (i = 0; i < (int)v.size(); ++i)
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (v[i] & 0xFF);
        if ((i + 1) % 16 == 0)
            std::cout << ' ';
    }
    std::cout << std::dec;
}

// ----------------------------------------------------------------
// Main – generate edge‑case inputs, encrypt, and display results
// ----------------------------------------------------------------
int main()
{
    // fixed key (16 bytes) – varied values
    std::vector<int> key = {
        0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
        0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F};

    // fixed nonce (12 bytes) – varied values
    std::vector<int> nonce = {
        0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,
        0xAA,0xAA,0xAA,0xAA,0xAA,0x02};

    OCB128 ocb(key);

    // test vectors with varied lengths and contents
    std::vector< std::vector<int> > tests;

    // empty plaintext
    tests.push_back(std::vector<int>());

    // 2‑byte plaintext
    tests.push_back(std::vector<int>{0x01, 0x02});

    // 15‑byte plaintext (non‑aligned)
    std::vector<int> blk15;
    for (int i = 0x10; i < 0x1F; ++i) blk15.push_back(i);
    tests.push_back(blk15);

    // exactly one block (16 bytes) with pattern
    std::vector<int> blk16(16);
    for (int i = 0; i < 16; ++i) blk16[i] = (i * 5) & 0xFF;
    tests.push_back(blk16);

    // 18 bytes (one full block + 2 bytes)
    std::vector<int> blk18 = blk16;
    blk18.push_back(0xAA);
    blk18.push_back(0xBB);
    tests.push_back(blk18);

    // 30 bytes (one full block + 14 bytes)
    std::vector<int> blk30;
    for (int i = 0; i < 30; ++i) blk30.push_back((i * 7) & 0xFF);
    tests.push_back(blk30);

    // 31 bytes (two blocks - 1 byte)
    std::vector<int> blk31 = blk30;
    blk31.push_back(0xCC);
    tests.push_back(blk31);

    // 64 bytes (four full blocks)
    std::vector<int> blk64;
    for (int i = 0; i < 64; ++i) blk64.push_back((i * 11) & 0xFF);
    tests.push_back(blk64);

    // process each test case
    int caseNo = 1;
    for (auto &plain : tests)
    {
        std::cout << "Case " << caseNo << " – plaintext length: " << plain.size() << "\n";

        std::cout << "Plaintext : ";
        printHex(plain);
        std::cout << "\n";

        std::vector<int> cipher;
        std::vector<int> tag;
        ocb.encrypt(nonce, plain, cipher, tag);

        std::cout << "Ciphertext: ";
        printHex(cipher);
        std::cout << "\n";

        std::cout << "Tag       : ";
        printHex(tag);
        std::cout << "\n\n";

        ++caseNo;
    }

    return 0;
}
