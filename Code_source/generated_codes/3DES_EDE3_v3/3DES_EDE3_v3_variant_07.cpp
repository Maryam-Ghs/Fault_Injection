#include <iostream>
#include <iomanip>

/* LLM input variant 7: reverse-adversarial */

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
        4,12,20,28,5,13,21,
        29,37,45,53,61,6,14,
        22,30,38,46,54,62,7,
        15,23,31,39,47,55,63,
        36,44,52,60,3,11,19,
        27,35,43,51,59,2,10,
        18,26,34,42,50,58,1,
        9,17,25,33,41,49,57
    };

    int PC2[48] = {
        32,29,36,50,42,46,
        53,34,56,39,49,44,
        48,33,45,51,40,30,
        55,47,37,31,52,41,
        2,13,20,27,7,16,
        8,26,4,12,19,23,
        10,21,6,15,28,3,
        5,1,24,11,17,14
    };

    int rotSchedule[16] = {
        1,2,2,2,2,2,2,1,
        2,2,2,2,2,2,1,1
    };

    int IP[64] = {
        7,15,23,31,39,47,55,63,
        5,13,21,29,37,45,53,61,
        3,11,19,27,35,43,51,59,
        1,9,17,25,33,41,49,57,
        8,16,24,32,40,48,56,64,
        6,14,22,30,38,46,54,62,
        4,12,20,28,36,44,52,60,
        2,10,18,26,34,42,50,58
    };

    int FP[64] = {
        25,57,17,49,9,41,1,33,
        26,58,18,50,10,42,2,34,
        27,59,19,51,11,43,3,35,
        28,60,20,52,12,44,4,36,
        29,61,21,53,13,45,5,37,
        30,62,22,54,14,46,6,38,
        31,63,23,55,15,47,7,39,
        32,64,24,56,16,48,8,40
    };

    int E[48] = {
        1,32,31,30,29,28,
        29,28,27,26,25,24,
        25,24,23,22,21,20,
        21,20,17,16,17,16,
        15,14,13,12,13,12,
        11,10,9,8,9,8,
        5,4,5,4,3,2,
        1,32,31,30,29,28
    };

    int P[32] = {
        25,4,11,22,
        6,30,13,19,
        9,3,27,32,
        14,24,8,2,
        10,31,18,5,
        26,23,15,1,
        17,28,12,29,
        21,20,7,16
    };

    // S‑boxes (8 boxes, each 4×16)
    int Sbox[8][64] = {
        // S1 reversed
        {13,0,6,10,4,14,9,2,15,12,8,1,3,5,7,11,
         0,5,9,15,6,3,12,13,8,2,10,4,14,1,11,7,
         13,3,10,5,0,8,6,2,12,9,1,14,4,7,11,15,
         6,9,15,13,2,5,3,10,0,11,4,8,7,1,14,12},
        // S2 reversed
        {9,14,0,5,12,3,11,1,6,2,13,4,15,8,10,7,
         5,9,10,13,4,2,14,15,3,6,11,12,7,0,1,8,
         13,4,2,6,9,0,1,11,5,15,14,3,7,12,8,10,
         1,15,6,3,10,8,5,2,14,13,9,0,12,7,4,11},
        // S3 reversed
        {12,2,3,8,14,11,13,0,6,4,15,9,5,10,1,7,
         0,5,12,4,13,9,8,6,2,7,14,15,3,11,10,1,
         7,12,1,6,13,8,5,9,3,15,0,4,2,14,10,11,
         13,14,2,3,10,5,1,0,11,7,9,12,8,6,15,4},
        // S4 reversed
        {14,2,4,15,9,12,7,5,0,1,6,10,8,3,13,11,
         9,4,11,13,5,2,8,14,3,7,6,1,15,0,12,10,
         4,0,9,8,5,13,2,14,7,3,10,12,15,6,1,11,
         12,6,3,15,0,5,10,2,13,14,8,4,7,1,9,11},
        // S5 reversed
        {9,14,0,3,6,5,15,4,8,12,13,2,11,7,1,10,
         6,9,12,8,15,2,5,13,7,11,4,14,1,0,3,10,
         0,14,6,9,13,8,5,3,2,12,15,4,7,11,10,1,
         4,15,1,10,12,6,9,13,5,2,8,11,14,0,3,7},
        // S6 reversed
        {13,8,6,0,5,2,14,11,7,15,3,1,12,4,10,9,
         2,5,13,11,6,9,8,12,4,3,0,7,15,10,14,1,
         10,7,0,1,12,14,9,6,3,13,8,4,2,15,5,11,
         11,4,7,2,13,8,6,5,14,12,1,9,15,3,0,10},
        // S7 reversed
        {12,3,6,2,15,14,0,5,9,10,8,4,11,1,7,13,
         2,8,15,5,6,9,13,11,7,10,12,3,0,4,14,1,
         4,12,9,3,7,14,2,1,8,11,6,15,5,0,13,10,
         6,8,5,4,10,13,7,11,14,2,9,1,12,15,3,0},
        // S8 reversed
        {11,6,5,3,0,9,12,15,13,8,10,14,4,2,7,1,
         2,9,5,0,12,1,14,7,6,13,8,11,15,4,10,3,
         8,5,3,12,11,6,15,1,4,10,2,9,13,14,7,0,
         7,1,0,5,4,10,8,2,13,12,9,15,3,11,14,6}
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
// Main – edge‑case heavy test vectors
// ------------------------------------------------------------
int main()
{
    // Edge‑case plaintexts (reversed pattern)
    int plainZero[8] = {0,0,0,0,0,0,0,0};
    int plainOnes[8] = {255,255,255,255,255,255,255,255};
    int plainPattern[8] = {0x55,0xAA,0x55,0xAA,0x55,0xAA,0x55,0xAA};

    // Edge‑case keys (three independent keys, reversed mixed key)
    int keyAllZero[8] = {0,0,0,0,0,0,0,0};
    int keyAllOne[8]  = {255,255,255,255,255,255,255,255};
    int keyMixed[8]   = {0xF1,0xDF,0xBC,0x9B,0x79,0x57,0x34,0x13};

    // Build three‑key sets
    TripleDES tdes1(keyAllZero, keyAllZero, keyAllZero);
    TripleDES tdes2(keyAllOne,  keyAllOne,  keyAllOne);
    TripleDES tdes3(keyMixed,   keyMixed,   keyMixed);

    int out[8];

    std::cout << "3DES-EDE3 (Version #3) – Edge case tests\n";

    // Test 1: all zero plaintext with all zero keys
    tdes1.encryptBlock(plainZero, out);
    std::cout << "ZeroPlain + ZeroKey : ";
    printHex(out);

    // Test 2: all ones plaintext with all ones keys
    tdes2.encryptBlock(plainOnes, out);
    std::cout << "OnesPlain + OnesKey : ";
    printHex(out);

    // Test 3: pattern plaintext with mixed (reversed) keys
    tdes3.encryptBlock(plainPattern, out);
    std::cout << "PatternPlain + MixedKey : ";
    printHex(out);

    // Additional cross‑combination
    tdes1.encryptBlock(plainPattern, out);
    std::cout << "PatternPlain + ZeroKey : ";
    printHex(out);

    tdes2.encryptBlock(plainZero, out);
    std::cout << "ZeroPlain + OnesKey : ";
    printHex(out);

    return 0;
}
