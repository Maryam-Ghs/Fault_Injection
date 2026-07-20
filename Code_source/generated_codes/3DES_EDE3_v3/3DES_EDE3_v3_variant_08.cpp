/* LLM input variant 8: sparse-skewed */
#include <iostream>
#include <iomanip>

// ------------------------------------------------------------
// 3DES-EDE3 implementation (Version #3)
// ------------------------------------------------------------
//  Only int and float are used (no double, long, unsigned, const)
//  All data structures are on the stack
//  Heavy use of loops and temporary variables
// ------------------------------------------------------------

class TripleDES
{
    // 16 sub‑keys for each DES key (each sub‑key = two 32‑bit halves)
    int subK1[16][2];
    int subK2[16][2];
    int subK3[16][2];

    // ------------------- DES tables (global, non‑const) ----------
    int PC1[56] = {
        57,49,41,33,25,17,9,
        1,58,50,42,34,26,18,
        10,2,59,51,43,35,27,
        19,11,3,60,52,44,36,
        63,55,47,39,31,23,15,
        7,62,54,46,38,30,22,
        14,6,61,53,45,37,29,
        21,13,5,28,20,12,4
    };

    int PC2[48] = {
        14,17,11,24,1,5,
        3,28,15,6,21,10,
        23,19,12,4,26,8,
        16,7,27,20,13,2,
        41,52,31,37,47,55,
        30,40,51,45,33,48,
        44,49,39,56,34,53,
        46,42,50,36,29,32
    };

    int rotSchedule[16] = {
        1,1,2,2,2,2,2,2,
        1,2,2,2,2,2,2,1
    };

    int IP[64] = {
        58,50,42,34,26,18,10,2,
        60,52,44,36,28,20,12,4,
        62,54,46,38,30,22,14,6,
        64,56,48,40,32,24,16,8,
        57,49,41,33,25,17,9,1,
        59,51,43,35,27,19,11,3,
        61,53,45,37,29,21,13,5,
        63,55,47,39,31,23,15,7
    };

    int FP[64] = {
        40,8,48,16,56,24,64,32,
        39,7,47,15,55,23,63,31,
        38,6,46,14,54,22,62,30,
        37,5,45,13,53,21,61,29,
        36,4,44,12,52,20,60,28,
        35,3,43,11,51,19,59,27,
        34,2,42,10,50,18,58,26,
        33,1,41,9,49,17,57,25
    };

    int E[48] = {
        32,1,2,3,4,5,
        4,5,6,7,8,9,
        8,9,10,11,12,13,
        12,13,14,15,16,17,
        16,17,18,19,20,21,
        20,21,22,23,24,25,
        24,25,26,27,28,29,
        28,29,30,31,32,1
    };

    int P[32] = {
        16,7,20,21,
        29,12,28,17,
        1,15,23,26,
        5,18,31,10,
        2,8,24,14,
        32,27,3,9,
        19,13,30,6,
        22,11,4,25
    };

    // S‑boxes (8 boxes, each 4×16)
    int Sbox[8][64] = {
        // S1
        {14,4,13,1,2,15,11,8,3,10,6,12,5,9,0,7,
         0,15,7,4,14,2,13,1,10,6,12,11,9,5,3,8,
         4,1,14,8,13,6,2,11,15,12,9,7,3,10,5,0,
         15,12,8,2,4,9,1,7,5,11,3,14,10,0,6,13},
        // S2
        {15,1,8,14,6,11,3,4,9,7,2,13,12,0,5,10,
         3,13,4,7,15,2,8,14,12,0,1,10,6,9,11,5,
         0,14,7,11,10,4,13,1,5,8,12,6,9,3,2,15,
         13,8,10,1,3,15,4,2,11,6,7,12,0,5,14,9},
        // S3
        {10,0,9,14,6,3,15,5,1,13,12,7,11,4,2,8,
         13,7,0,9,3,4,6,10,2,8,5,14,12,11,15,1,
         13,6,4,9,8,15,3,0,11,1,2,12,5,10,14,7,
         1,10,13,0,6,9,8,7,4,15,14,3,11,5,2,12},
        // S4
        {7,13,14,3,0,6,9,10,1,2,8,5,11,12,4,15,
         13,8,11,5,6,15,0,3,4,7,2,12,1,10,14,9,
         10,6,9,0,12,11,7,13,15,1,3,14,5,2,8,4,
         3,15,0,6,10,1,13,8,9,4,5,11,12,7,2,14},
        // S5
        {2,12,4,1,7,10,11,6,8,5,3,15,13,0,14,9,
         14,11,2,12,4,7,13,1,5,0,15,10,3,9,8,6,
         4,2,1,11,10,13,7,8,15,9,12,5,6,3,0,14,
         11,8,12,7,1,14,2,13,6,15,0,9,10,4,5,3},
        // S6
        {12,1,10,15,9,2,6,8,0,13,3,4,14,7,5,11,
         10,15,4,2,7,12,9,5,6,1,13,14,0,11,3,8,
         9,14,15,5,2,8,12,3,7,0,4,10,1,13,11,6,
         4,3,2,12,9,5,15,10,11,14,1,7,6,0,8,13},
        // S7
        {4,11,2,14,15,0,8,13,3,12,9,7,5,10,6,1,
         13,0,11,7,4,9,1,10,14,3,5,12,2,15,8,6,
         1,4,11,13,12,3,7,14,10,15,6,8,0,5,9,2,
         6,11,13,8,1,4,10,7,9,5,0,15,14,2,3,12},
        // S8
        {13,2,8,4,6,15,11,1,10,9,3,14,5,0,12,7,
         1,15,13,8,10,3,7,4,12,5,6,11,0,14,9,2,
         7,11,4,1,9,12,14,2,0,6,10,13,15,3,5,8,
         2,1,14,7,4,10,8,13,15,12,9,0,3,5,6,11}
    };

    // ------------------------------------------------------------
    // Helper: apply a permutation table to a 64‑bit block
    // ------------------------------------------------------------
    void permuteBlock(const int in[64], int out[64], const int *table, int n)
    {
        for (int i = 0; i < n; ++i)
        {
            out[i] = in[table[i] - 1];
        }
    }

    // ------------------------------------------------------------
    // Helper: left‑rotate a 28‑bit half‑key
    // ------------------------------------------------------------
    void rotate28(int half[28], int steps)
    {
        int tmp[28];
        for (int i = 0; i < 28; ++i)
            tmp[i] = half[(i + steps) % 28];
        for (int i = 0; i < 28; ++i)
            half[i] = tmp[i];
    }

    // ------------------------------------------------------------
    // Key schedule for a single 64‑bit key (8‑byte array)
    // ------------------------------------------------------------
    void generateSubKeys(const int keyBytes[8], int sub[16][2])
    {
        int keyBits[64];
        // Convert key bytes to bits
        for (int i = 0; i < 8; ++i)
        {
            for (int b = 0; b < 8; ++b)
                keyBits[i * 8 + b] = (keyBytes[i] >> (7 - b)) & 1;
        }

        // PC‑1
        int perm1[56];
        permuteBlock(keyBits, perm1, PC1, 56);

        // Split into C and D (28 bits each)
        int C[28], D[28];
        for (int i = 0; i < 28; ++i)
        {
            C[i] = perm1[i];
            D[i] = perm1[i + 28];
        }

        // 16 rounds
        for (int round = 0; round < 16; ++round)
        {
            rotate28(C, rotSchedule[round]);
            rotate28(D, rotSchedule[round]);

            // Combine C and D
            int CD[56];
            for (int i = 0; i < 28; ++i)
            {
                CD[i] = C[i];
                CD[i + 28] = D[i];
            }

            // PC‑2 -> 48‑bit sub‑key
            int subBits[48];
            permuteBlock(CD, subBits, PC2, 48);

            // Pack sub‑key into two 32‑bit ints (only lower 24 bits used in each)
            int left = 0, right = 0;
            for (int i = 0; i < 24; ++i)
                left = (left << 1) | subBits[i];
            for (int i = 24; i < 48; ++i)
                right = (right << 1) | subBits[i];

            sub[round][0] = left;
            sub[round][1] = right;
        }
    }

    // ------------------------------------------------------------
    // The core DES round function (F)
    // ------------------------------------------------------------
    int feistel(int R, const int *subKey)
    {
        // Expansion (32 -> 48)
        int Rbits[32];
        for (int i = 0; i < 32; ++i)
            Rbits[i] = (R >> (31 - i)) & 1;

        int Eb[48];
        for (int i = 0; i < 48; ++i)
            Eb[i] = Rbits[E[i] - 1];

        // XOR with sub‑key
        int subBits[48];
        for (int i = 0; i < 24; ++i)
            subBits[i] = (subKey[0] >> (23 - i)) & 1;
        for (int i = 0; i < 24; ++i)
            subBits[i + 24] = (subKey[1] >> (23 - i)) & 1;

        for (int i = 0; i < 48; ++i)
            Eb[i] ^= subBits[i];

        // S‑box substitution (32 bits)
        int sOut[32];
        for (int box = 0; box < 8; ++box)
        {
            int row = (Eb[box * 6] << 1) | Eb[box * 6 + 5];
            int col = (Eb[box * 6 + 1] << 3) |
                      (Eb[box * 6 + 2] << 2) |
                      (Eb[box * 6 + 3] << 1) |
                      Eb[box * 6 + 4];
            int sVal = Sbox[box][row * 16 + col];
            for (int bit = 0; bit < 4; ++bit)
                sOut[box * 4 + bit] = (sVal >> (3 - bit)) & 1;
        }

        // Permutation P
        int pOut[32];
        for (int i = 0; i < 32; ++i)
            pOut[i] = sOut[P[i] - 1];

        // Pack back into int
        int result = 0;
        for (int i = 0; i < 32; ++i)
            result = (result << 1) | pOut[i];

        return result;
    }

    // ------------------------------------------------------------
    // Single‑key DES encrypt/decrypt (encrypt if enc=1, decrypt if enc=0)
    // ------------------------------------------------------------
    void desCore(const int in[64], int out[64], const int sub[16][2], int enc)
    {
        // Initial permutation
        int ip[64];
        permuteBlock(in, ip, IP, 64);

        // Split into L and R (32 bits each)
        int L = 0, R = 0;
        for (int i = 0; i < 32; ++i)
            L = (L << 1) | ip[i];
        for (int i = 32; i < 64; ++i)
            R = (R << 1) | ip[i];

        // 16 rounds
        for (int round = 0; round < 16; ++round)
        {
            int subIdx = enc ? round : 15 - round;
            int fOut = feistel(R, sub[subIdx]);
            int newR = L ^ fOut;
            L = R;
            R = newR;
        }

        // Pre‑output (swap L and R)
        int pre[64];
        for (int i = 0; i < 32; ++i)
        {
            pre[i] = (R >> (31 - i)) & 1;
            pre[i + 32] = (L >> (31 - i)) & 1;
        }

        // Final permutation
        permuteBlock(pre, out, FP, 64);
    }

public:
    // ------------------------------------------------------------
    // Constructor – builds sub‑keys for three keys
    // ------------------------------------------------------------
    TripleDES(const int key1[8], const int key2[8], const int key3[8])
    {
        generateSubKeys(key1, subK1);
        generateSubKeys(key2, subK2);
        generateSubKeys(key3, subK3);
    }

    // ------------------------------------------------------------
    // 3DES encrypt (EDE)
    // ------------------------------------------------------------
    void encryptBlock(const int plain[8], int cipher[8])
    {
        // Convert plaintext bytes to bit array
        int bits[64];
        for (int i = 0; i < 8; ++i)
            for (int b = 0; b < 8; ++b)
                bits[i * 8 + b] = (plain[i] >> (7 - b)) & 1;

        int after1[64], after2[64], after3[64];

        // 1st DES encrypt
        desCore(bits, after1, subK1, 1);
        // 2nd DES decrypt
        desCore(after1, after2, subK2, 0);
        // 3rd DES encrypt
        desCore(after2, after3, subK3, 1);

        // Pack bits back to bytes
        for (int i = 0; i < 8; ++i)
        {
            int val = 0;
            for (int b = 0; b < 8; ++b)
                val = (val << 1) | after3[i * 8 + b];
            cipher[i] = val;
        }
    }
};

// ------------------------------------------------------------
// Helper: print an 8‑byte block as hex
// ------------------------------------------------------------
void printHex(const int block[8])
{
    for (int i = 0; i < 8; ++i)
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (block[i] & 0xFF);
    std::cout << std::dec << std::endl;
}

// ------------------------------------------------------------
// Main – sparse‑skewed test vectors
// ------------------------------------------------------------
int main()
{
    // Sparse plaintexts (mostly zeros, few bits set)
    int plainSparse1[8] = {0x01, 0, 0, 0, 0, 0, 0, 0};          // single LSB set
    int plainSparse2[8] = {0x80, 0, 0, 0, 0, 0, 0, 0};          // single MSB set
    int plainSparse3[8] = {0x00, 0x00, 0x10, 0x00, 0x00, 0x04, 0x00, 0x00}; // scattered bits

    // Sparse keys (mostly zeros, isolated bits)
    int keySparseA[8] = {0,0,0,0,0,0,0,0x01};   // only LSB of last byte
    int keySparseB[8] = {0x80,0,0,0,0,0,0,0};   // only MSB of first byte
    int keySparseC[8] = {0,0,0,0,0,0,0xF0,0};   // half‑byte set in seventh position

    // Build three‑key sets
    TripleDES tdesZero(keySparseA, keySparseA, keySparseA); // all three keys identical sparse A
    TripleDES tdesMixed(keySparseB, keySparseC, keySparseA);
    TripleDES tdesAlt(keySparseC, keySparseB, keySparseC);

    int out[8];

    std::cout << "3DES-EDE3 (Version #3) – Sparse‑Skewed tests\n";

    // Test 1: sparse plaintext with sparse key set A
    tdesZero.encryptBlock(plainSparse1, out);
    std::cout << "SparsePlain1 + SparseKeyA : ";
    printHex(out);

    // Test 2: another sparse plaintext with same key set A
    tdesZero.encryptBlock(plainSparse2, out);
    std::cout << "SparsePlain2 + SparseKeyA : ";
    printHex(out);

    // Test 3: scattered bits plaintext with mixed key set
    tdesMixed.encryptBlock(plainSparse3, out);
    std::cout << "SparsePlain3 + MixedKeys : ";
    printHex(out);

    // Additional cross‑combination
    tdesAlt.encryptBlock(plainSparse1, out);
    std::cout << "SparsePlain1 + MixedKeysAlt : ";
    printHex(out);

    tdesMixed.encryptBlock(plainSparse2, out);
    std::cout << "SparsePlain2 + MixedKeys : ";
    printHex(out);

    return 0;
}
