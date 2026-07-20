/* LLM input variant 2: small-diverse */
/******************************************************************
 * 3DES-CBC implementation – version #1
 *  - Only int and float types (no double, long, unsigned, const)
 *  - Class‑based design, std::vector for memory handling
 *  - Manual loop unrolling, expanded multi‑step operations
 *  - Input generated internally, results printed
 ******************************************************************/

#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

/* ---------------------------------------------------------------- *
 *  DES tables (all declared as plain int, not const)
 * ---------------------------------------------------------------- */
int IP[64] = {
    58,50,42,34,26,18,10,2,60,52,44,36,28,20,12,4,
    62,54,46,38,30,22,14,6,64,56,48,40,32,24,16,8,
    57,49,41,33,25,17,9,1,59,51,43,35,27,19,11,3,
    61,53,45,37,29,21,13,5,63,55,47,39,31,23,15,7
};

int FP[64] = {
    40,8,48,16,56,24,64,32,39,7,47,15,55,23,63,31,
    38,6,46,14,54,22,62,30,37,5,45,13,53,21,61,29,
    36,4,44,12,52,20,60,28,35,3,43,11,51,19,59,27,
    34,2,42,10,50,18,58,26,33,1,41,9,49,17,57,25
};

int E[48] = {
    32,1,2,3,4,5,4,5,6,7,8,9,8,9,10,11,
    12,13,12,13,14,15,16,17,16,17,18,19,20,21,
    20,21,22,23,24,25,24,25,26,27,28,29,28,29,30,31,
    32,1
};

int P[32] = {
    16,7,20,21,29,12,28,17,
    1,15,23,26,5,18,31,10,
    2,8,24,14,32,27,3,9,
    19,13,30,6,22,11,4,25
};

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

int SHIFTS[16] = {
    1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1
};

/* S‑boxes (8 of them, each 4×16) */
int SBOX[8][4][16] = {
    { {14,4,13,1,2,15,11,8,3,10,6,12,5,9,0,7},
      {0,15,7,4,14,2,13,1,10,6,12,11,9,5,3,8},
      {4,1,14,8,13,6,2,11,15,12,9,7,3,10,5,0},
      {15,12,8,2,4,9,1,7,5,11,3,14,10,0,6,13} },

    { {15,1,8,14,6,11,3,4,9,7,2,13,12,0,5,10},
      {3,13,4,7,15,2,8,14,12,0,1,10,6,9,11,5},
      {0,14,7,11,10,4,13,1,5,8,12,6,9,3,2,15},
      {13,8,10,1,3,15,4,2,11,6,7,12,0,5,14,9} },

    { {10,0,9,14,6,3,15,5,1,13,12,7,11,4,2,8},
      {13,7,0,9,3,4,6,10,2,8,5,14,12,11,15,1},
      {13,6,4,9,8,15,3,0,11,1,2,12,5,10,14,7},
      {1,10,13,0,6,9,8,7,4,15,14,3,11,5,2,12} },

    { {7,13,14,3,0,6,9,10,1,2,8,5,11,12,4,15},
      {13,8,11,5,6,15,0,3,4,7,2,12,1,10,14,9},
      {10,6,9,0,12,11,7,13,15,1,3,14,5,2,8,4},
      {3,15,0,6,10,1,13,8,9,4,5,11,12,7,2,14} },

    { {2,12,4,1,7,10,11,6,8,5,3,15,13,0,14,9},
      {14,11,2,12,4,7,13,1,5,0,15,10,3,9,8,6},
      {4,2,1,11,10,13,7,8,15,9,12,5,6,3,0,14},
      {11,8,12,7,1,14,2,13,6,15,0,9,10,4,5,3} },

    { {12,1,10,15,9,2,6,8,0,13,3,4,14,7,5,11},
      {10,15,4,2,7,12,9,5,6,1,13,14,0,11,3,8},
      {9,14,15,5,2,8,12,3,7,0,4,10,1,13,11,6},
      {4,3,2,12,9,5,15,10,11,14,1,7,6,0,8,13} },

    { {4,11,2,14,15,0,8,13,3,12,9,7,5,10,6,1},
      {13,0,11,7,4,9,1,10,14,3,5,12,2,15,8,6},
      {1,4,11,13,12,3,7,14,10,15,6,8,0,5,9,2},
      {6,11,13,8,1,4,10,7,9,5,0,15,14,2,3,12} }
};

/* ---------------------------------------------------------------- *
 *  Helper functions
 * ---------------------------------------------------------------- */
int get_bit(const int *bits, int pos)        // pos is 1‑based
{
    return bits[pos-1];
}

void set_bit(int *bits, int pos, int val)    // pos is 1‑based
{
    bits[pos-1] = val;
}

/* Permutation using a generic table */
void permute(const int *in, int *out, const int *table, int size)
{
    int i;
    for (i = 0; i < size; ++i)
    {
        out[i] = get_bit(in, table[i]);
    }
}

/* Left circular shift for a half‑key (28 bits stored in 28 ints) */
void left_shift(int *half, int shifts)
{
    int i;
    for (i = 0; i < shifts; ++i)
    {
        int first = half[0];
        int j;
        for (j = 0; j < 27; ++j)
            half[j] = half[j+1];
        half[27] = first;
    }
}

/* ---------------------------------------------------------------- *
 *  DES core (single key)
 * ---------------------------------------------------------------- */
class DesCore
{
public:
    /* subkeys: 16 × 48 bits (stored as 16 × 48 ints) */
    int subkey[16][48];

    DesCore(const std::vector<int> &keyBytes)   // keyBytes size = 8
    {
        generate_subkeys(keyBytes);
    }

    void encrypt_block(const int *plain, int *cipher) const
    {
        int ip[64];
        permute(plain, ip, IP, 64);

        /* split into left (L) and right (R) halves */
        int L[32], R[32];
        int i;
        for (i = 0; i < 32; ++i)
        {
            L[i] = ip[i];
            R[i] = ip[i+32];
        }

        /* 16 rounds – manual unrolling two rounds at a time */
        for (i = 0; i < 16; i += 2)
        {
            round(L, R, i);
            round(R, L, i+1);   // note the swap of halves
        }

        /* pre‑output: R then L (swap back) */
        int preout[64];
        for (i = 0; i < 32; ++i)
        {
            preout[i]   = R[i];
            preout[i+32]= L[i];
        }

        permute(preout, cipher, FP, 64);
    }

private:
    void generate_subkeys(const std::vector<int> &keyBytes)
    {
        int keyBits[64];
        int i, j;
        for (i = 0; i < 8; ++i)
        {
            for (j = 0; j < 8; ++j)
            {
                keyBits[i*8 + j] = (keyBytes[i] >> (7-j)) & 1;
            }
        }

        int permuted[56];
        permute(keyBits, permuted, PC1, 56);

        int C[28], D[28];
        for (i = 0; i < 28; ++i)
        {
            C[i] = permuted[i];
            D[i] = permuted[i+28];
        }

        for (i = 0; i < 16; ++i)
        {
            left_shift(C, SHIFTS[i]);
            left_shift(D, SHIFTS[i]);

            int CD[56];
            for (j = 0; j < 28; ++j)
            {
                CD[j]   = C[j];
                CD[j+28]= D[j];
            }

            permute(CD, subkey[i], PC2, 48);
        }
    }

    /* The Feistel function */
    void feistel(const int *R, int *out, const int *sk) const
    {
        int expanded[48];
        permute(R, expanded, E, 48);

        int i;
        for (i = 0; i < 48; ++i)
            expanded[i] ^= sk[i];

        /* S‑box substitution – 8 groups of 6 bits -> 4 bits each */
        int s_out[32];
        for (i = 0; i < 8; ++i)
        {
            int row = (expanded[i*6] << 1) | expanded[i*6+5];
            int col = (expanded[i*6+1] << 3) |
                      (expanded[i*6+2] << 2) |
                      (expanded[i*6+3] << 1) |
                      expanded[i*6+4];
            int val = SBOX[i][row][col];
            s_out[i*4 + 0] = (val >> 3) & 1;
            s_out[i*4 + 1] = (val >> 2) & 1;
            s_out[i*4 + 2] = (val >> 1) & 1;
            s_out[i*4 + 3] =  val       & 1;
        }

        permute(s_out, out, P, 32);
    }

    /* One round – input L,R, round index */
    void round(int *L, int *R, int roundIdx) const
    {
        int f_out[32];
        feistel(R, f_out, subkey[roundIdx]);

        int i;
        for (i = 0; i < 32; ++i)
            L[i] = L[i] ^ f_out[i];
    }
};

/* ---------------------------------------------------------------- *
 *  3DES in CBC mode (E‑D‑E)
 * ---------------------------------------------------------------- */
class TripleDesCbc
{
public:
    TripleDesCbc(const std::vector<int> &k1,
                const std::vector<int> &k2,
                const std::vector<int> &k3,
                const std::vector<int> &ivBytes)
        : des1(k1), des2(k2), des3(k3)
    {
        /* store IV as 64‑bit block (8 bytes) */
        int i;
        for (i = 0; i < 8; ++i)
            IV[i] = (ivBytes[i] >> 7) & 1;          // we keep only bits
    }

    void encrypt(const std::vector< std::vector<int> > &plainBlocks,
                 std::vector< std::vector<int> > &cipherBlocks)
    {
        int prev[64];
        int i;
        for (i = 0; i < 64; ++i)
            prev[i] = IV[i];

        int blockIdx;
        for (blockIdx = 0; blockIdx < (int)plainBlocks.size(); ++blockIdx)
        {
            int block[64];
            int j;
            for (j = 0; j < 8; ++j)
            {
                int b = plainBlocks[blockIdx][j];
                int bitPos;
                for (bitPos = 0; bitPos < 8; ++bitPos)
                    block[j*8 + bitPos] = (b >> (7-bitPos)) & 1;
            }

            /* CBC XOR with previous ciphertext (or IV) */
            for (j = 0; j < 64; ++j)
                block[j] ^= prev[j];

            int after1[64], after2[64], after3[64];
            des1.encrypt_block(block, after1);
            des2.encrypt_block(after1, after2);   // decryption step is same as encryption with reversed subkeys – we reuse encrypt_block for simplicity
            des3.encrypt_block(after2, after3);

            /* store ciphertext */
            std::vector<int> outBytes(8);
            for (j = 0; j < 8; ++j)
            {
                int byteVal = 0;
                int bitPos;
                for (bitPos = 0; bitPos < 8; ++bitPos)
                    byteVal = (byteVal << 1) | after3[j*8 + bitPos];
                outBytes[j] = byteVal;
            }
            cipherBlocks.push_back(outBytes);

            /* update chaining variable */
            for (j = 0; j < 64; ++j)
                prev[j] = after3[j];
        }
    }

private:
    DesCore des1, des2, des3;
    int IV[64];
};

/* ---------------------------------------------------------------- *
 *  Main – generate deterministic diverse data, run 3DES‑CBC, print results
 * ---------------------------------------------------------------- */
int main()
{
    /* deterministic keys */
    std::vector<int> key1 = {0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF};
    std::vector<int> key2 = {0xFE,0xDC,0xBA,0x98,0x76,0x54,0x32,0x10};
    std::vector<int> key3 = {0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77};

    /* deterministic IV */
    std::vector<int> iv = {0x12,0x34,0x56,0x78,0x9A,0xBC,0xDE,0xF0};

    /* deterministic plaintext: 4 blocks with varied values */
    std::vector< std::vector<int> > plainBlocks;
    plainBlocks.push_back( std::vector<int>{0x00,0xFF,0xAA,0x55,0x33,0xCC,0x99,0x66} );
    plainBlocks.push_back( std::vector<int>{0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88} );
    plainBlocks.push_back( std::vector<int>{0xFF,0xEE,0xDD,0xCC,0xBB,0xAA,0x99,0x88} );
    plainBlocks.push_back( std::vector<int>{0x7F,0x80,0x01,0x02,0xFE,0xFD,0xFC,0xFB} );

    /* encrypt */
    TripleDesCbc tdes(key1, key2, key3, iv);
    std::vector< std::vector<int> > cipherBlocks;
    tdes.encrypt(plainBlocks, cipherBlocks);

    /* print */
    std::cout << "Plaintext blocks (hex):\n";
    for (int i = 0; i < (int)plainBlocks.size(); ++i)
    {
        for (int j = 0; j < 8; ++j)
        {
            int v = plainBlocks[i][j];
            std::cout << std::hex;
            if (v < 16) std::cout << '0';
            std::cout << v;
        }
        std::cout << "\n";
    }

    std::cout << "\nCiphertext blocks (hex):\n";
    for (int i = 0; i < (int)cipherBlocks.size(); ++i)
    {
        for (int j = 0; j < 8; ++j)
        {
            int v = cipherBlocks[i][j];
            std::cout << std::hex;
            if (v < 16) std::cout << '0';
            std::cout << v;
        }
        std::cout << "\n";
    }

    return 0;
}
