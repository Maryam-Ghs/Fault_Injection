#include <iostream>

/* LLM input variant 1: minimal-boundary */

/* -------------------------------------------------------------
   3‑DES (EDE3) implementation – version #10
   Only int is used for all data.  All tables and buffers live on
   the stack.  The code is deliberately reorganised compared with
   earlier versions: variable names, loop structures and the order
   of operations are changed while preserving the mathematics.
   ------------------------------------------------------------- */

class Des3 {
public:
    // 8‑byte block (each byte stored in an int)
    typedef int Block[8];
    // 8‑byte key (each byte stored in an int)
    typedef int Key[8];

    // -----------------------------------------------------------------
    // Public interface
    // -----------------------------------------------------------------
    void encrypt(Block out, const Block in,
                 const Key k1, const Key k2, const Key k3)
    {
        Block tmp;
        encryptBlock(tmp, in, k1);   // ENC with K1
        decryptBlock(out, tmp, k2);  // DEC with K2
        encryptBlock(tmp, out, k3);  // ENC with K3
        copyBlock(out, tmp);
    }

private:
    // -----------------------------------------------------------------
    // DES core (single‑key) – encryption and decryption share code
    // -----------------------------------------------------------------
    void encryptBlock(Block out, const Block in, const Key key)
    {
        int sub[16][48];
        genSubKeys(sub, key);
        int left, right;
        permuteInitial(left, right, in);
        for (int round = 0; round < 16; ++round) {
            int f = feistel(right, sub[round]);
            int newLeft = right;
            int newRight = left ^ f;          // fused XOR
            left = newLeft;
            right = newRight;
        }
        permuteFinal(out, right, left);      // note swapped halves
    }

    void decryptBlock(Block out, const Block in, const Key key)
    {
        int sub[16][48];
        genSubKeys(sub, key);
        int left, right;
        permuteInitial(left, right, in);
        for (int round = 15; round >= 0; --round) {
            int f = feistel(right, sub[round]);
            int newLeft = right;
            int newRight = left ^ f;
            left = newLeft;
            right = newRight;
        }
        permuteFinal(out, right, left);
    }

    // -----------------------------------------------------------------
    // Initial permutation (IP) – produces two 32‑bit halves
    // -----------------------------------------------------------------
    void permuteInitial(int &L, int &R, const Block src)
    {
        static const int IP[64] = {
            58,50,42,34,26,18,10, 2, 60,52,44,36,28,20,12, 4,
            62,54,46,38,30,22,14, 6, 64,56,48,40,32,24,16, 8,
            57,49,41,33,25,17, 9, 1, 59,51,43,35,27,19,11, 3,
            61,53,45,37,29,21,13, 5, 63,55,47,39,31,23,15, 7 };
        L = R = 0;
        for (int i = 0; i < 32; ++i) {
            int bit = getBit(src, IP[i] - 1);
            L = (L << 1) | bit;                     // build left half
        }
        for (int i = 32; i < 64; ++i) {
            int bit = getBit(src, IP[i] - 1);
            R = (R << 1) | bit;                     // build right half
        }
    }

    // -----------------------------------------------------------------
    // Final permutation (IP⁻¹)
    // -----------------------------------------------------------------
    void permuteFinal(Block dst, int L, int R)
    {
        static const int FP[64] = {
            40, 8,48,16,56,24,64,32, 39,7,47,15,55,23,63,31,
            38,6,46,14,54,22,62,30, 37,5,45,13,53,21,61,29,
            36,4,44,12,52,20,60,28, 35,3,43,11,51,19,59,27,
            34,2,42,10,50,18,58,26, 33,1,41,9,49,17,57,25 };
        int combined[64];
        for (int i = 0; i < 32; ++i) combined[i] = (L >> (31 - i)) & 1;
        for (int i = 0; i < 32; ++i) combined[32 + i] = (R >> (31 - i)) & 1;

        for (int i = 0; i < 8; ++i) dst[i] = 0;
        for (int i = 0; i < 64; ++i) {
            int bit = combined[FP[i] - 1];
            dst[i / 8] = (dst[i / 8] << 1) | bit;
        }
    }

    // -----------------------------------------------------------------
    // Feistel function F(R, K)
    // -----------------------------------------------------------------
    int feistel(int R, const int *subKey)
    {
        static const int E[48] = {
            32, 1, 2, 3, 4, 5, 4, 5, 6, 7, 8, 9,
             8, 9,10,11,12,13,12,13,14,15,16,17,
            16,17,18,19,20,21,20,21,22,23,24,25,
            24,25,26,27,28,29,28,29,30,31,32,1 };
        static const int S[8][4][16] = {
            {{14,4,13,1,2,15,11,8,3,10,6,12,5,9,0,7},
             {0,15,7,4,14,2,13,1,10,6,12,11,9,5,3,8},
             {4,1,14,8,13,6,2,11,15,12,9,7,3,10,5,0},
             {15,12,8,2,4,9,1,7,5,11,3,14,10,0,6,13}},
            {{15,1,8,14,6,11,3,4,9,7,2,13,12,0,5,10},
             {3,13,4,7,15,2,8,14,12,0,1,10,6,9,11,5},
             {0,14,7,11,10,4,13,1,5,8,12,6,9,3,2,15},
             {13,8,10,1,3,15,4,2,11,6,7,12,0,5,14,9}},
            {{10,0,9,14,6,3,15,5,1,13,12,7,11,4,2,8},
             {13,7,0,9,3,4,6,10,2,8,5,14,12,11,15,1},
             {13,6,4,9,8,15,3,0,11,1,2,12,5,10,14,7},
             {1,10,13,0,6,9,8,7,4,15,14,3,11,5,2,12}},
            {{7,13,14,3,0,6,9,10,1,2,8,5,11,12,4,15},
             {13,8,11,5,6,15,0,3,4,7,2,12,1,10,14,9},
             {10,6,9,0,12,11,7,13,15,1,3,14,5,2,8,4},
             {3,15,0,6,10,1,13,8,9,4,5,11,12,7,2,14}},
            {{2,12,4,1,7,10,11,6,8,5,3,15,13,0,14,9},
             {14,11,2,12,4,7,13,1,5,0,15,10,3,9,8,6},
             {4,2,1,11,10,13,7,8,15,9,12,5,6,3,0,14},
             {11,8,12,7,1,14,2,13,6,15,0,9,10,4,5,3}},
            {{12,1,10,15,9,2,6,8,0,13,3,4,14,7,5,11},
             {10,15,4,2,7,12,9,5,6,1,13,14,0,11,3,8},
             {9,14,15,5,2,8,12,3,7,0,4,10,1,13,11,6},
             {4,3,2,12,9,5,15,10,11,14,1,7,6,0,8,13}},
            {{4,11,2,14,15,0,8,13,3,12,9,7,5,10,6,1},
             {13,0,11,7,4,9,1,10,14,3,5,12,2,15,8,6},
             {1,4,11,13,12,3,7,14,10,15,6,8,0,5,9,2},
             {6,11,13,8,1,4,10,7,9,5,0,15,14,2,3,12}},
            {{13,2,8,4,6,15,11,1,10,9,3,14,5,0,12,7},
             {1,15,13,8,10,3,7,4,12,5,6,11,0,14,9,2},
             {7,11,4,1,9,12,14,2,0,6,10,13,15,3,5,8},
             {2,1,14,7,4,10,8,13,15,12,9,0,3,5,6,11}}
        };
        static const int P[32] = {
            16,7,20,21,29,12,28,17,
             1,15,23,26,5,18,31,10,
             2,8,24,14,32,27,3,9,
            19,13,30,6,22,11,4,25 };

        // Expansion (32 → 48)
        int expanded = 0;
        for (int i = 0; i < 48; ++i) {
            int bit = (R >> (32 - E[i])) & 1;
            expanded = (expanded << 1) | bit;
        }

        // Key mixing
        int mixed = expanded ^ subKeyToInt(subKey);

        // S‑box substitution (48 → 32)
        int sOut = 0;
        for (int i = 0; i < 8; ++i) {
            int six = (mixed >> (42 - 6 * i)) & 0x3F;
            int row = ((six & 0x20) >> 4) | (six & 0x01);
            int col = (six >> 1) & 0x0F;
            int val = S[i][row][col];
            sOut = (sOut << 4) | val;
        }

        // Permutation P (32 → 32)
        int f = 0;
        for (int i = 0; i < 32; ++i) {
            int bit = (sOut >> (32 - P[i])) & 1;
            f = (f << 1) | bit;
        }
        return f;
    }

    // -----------------------------------------------------------------
    // Sub‑key generation (16 × 48‑bit)
    // -----------------------------------------------------------------
    void genSubKeys(int sub[16][48], const Key K)
    {
        static const int PC1[56] = {
            57,49,41,33,25,17,9,
             1,58,50,42,34,26,18,
            10,2,59,51,43,35,27,
            19,11,3,60,52,44,36,
            63,55,47,39,31,23,15,
             7,62,54,46,38,30,22,
            14,6,61,53,45,37,29,
            21,13,5,28,20,12,4 };
        static const int PC2[48] = {
            14,17,11,24,1,5,
            3,28,15,6,21,10,
            23,19,12,4,26,8,
            16,7,27,20,13,2,
            41,52,31,37,47,55,
            30,40,51,45,33,48,
            44,49,39,56,34,53,
            46,42,50,36,29,32 };
        static const int SHIFTS[16] = {
            1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1 };

        int c = 0, d = 0;
        // PC‑1 permutation (64 → 56)
        for (int i = 0; i < 28; ++i) {
            c = (c << 1) | getBit(K, PC1[i] - 1);
            d = (d << 1) | getBit(K, PC1[i + 28] - 1);
        }

        // 16 rounds
        for (int r = 0; r < 16; ++r) {
            // left rotations
            c = ((c << SHIFTS[r]) | (c >> (28 - SHIFTS[r]))) & 0x0FFFFFFF;
            d = ((d << SHIFTS[r]) | (d >> (28 - SHIFTS[r]))) & 0x0FFFFFFF;

            // PC‑2 permutation (56 → 48)
            int cd = (c << 28) | d;
            for (int i = 0; i < 48; ++i) {
                sub[r][i] = (cd >> (56 - PC2[i])) & 1;
            }
        }
    }

    // -----------------------------------------------------------------
    // Helper utilities – all use only int
    // -----------------------------------------------------------------
    int getBit(const Block src, int pos)
    {
        int byteIdx = pos / 8;
        int bitIdx  = 7 - (pos % 8);
        return (src[byteIdx] >> bitIdx) & 1;
    }

    int subKeyToInt(const int *bits)
    {
        int val = 0;
        for (int i = 0; i < 48; ++i) {
            val = (val << 1) | bits[i];
        }
        return val;
    }

    void copyBlock(Block dst, const Block src)
    {
        for (int i = 0; i < 8; ++i) dst[i] = src[i];
    }
};

int main()
{
    // -----------------------------------------------------------------
    // Minimal deterministic keys and plaintext (8‑byte arrays)
    // -----------------------------------------------------------------
    Des3::Key key1 = {0,0,0,0,0,0,0,0};
    Des3::Key key2 = {0,0,0,0,0,0,0,0};
    Des3::Key key3 = {0,0,0,0,0,0,0,0};
    Des3::Block plain = {0,0,0,0,0,0,0,1}; // smallest non‑zero plaintext
    Des3::Block cipher;

    // -----------------------------------------------------------------
    // Perform 3‑DES (EDE3)
    // -----------------------------------------------------------------
    Des3 engine;
    engine.encrypt(cipher, plain, key1, key2, key3);

    // -----------------------------------------------------------------
    // Print in hex
    // -----------------------------------------------------------------
    std::cout << "Plaintext : ";
    for (int i = 0; i < 8; ++i)
        std::cout << std::hex << (plain[i] & 0xFF) << (i == 7 ? "\n" : " ");

    std::cout << "Ciphertext: ";
    for (int i = 0; i < 8; ++i)
        std::cout << std::hex << (cipher[i] & 0xFF) << (i == 7 ? "\n" : " ");

    return 0;
}
