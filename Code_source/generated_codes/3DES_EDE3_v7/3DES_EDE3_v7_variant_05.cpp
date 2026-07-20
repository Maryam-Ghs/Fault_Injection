#include <iostream>
#include <vector>
#include <iomanip>

/* LLM input variant 5: duplicate-heavy */

///////////////////////////////////////////////////////////////////////////////
// DES tables (no const, only int)
///////////////////////////////////////////////////////////////////////////////

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

int Sbox[8][4][16] = {
    // S1
    {
        {14,4,13,1,2,15,11,8,3,10,6,12,5,9,0,7},
        {0,15,7,4,14,2,13,1,10,6,12,11,9,5,3,8},
        {4,1,14,8,13,6,2,11,15,12,9,7,3,10,5,0},
        {15,12,8,2,4,9,1,7,5,11,3,14,10,0,6,13}
    },
    // S2
    {
        {15,1,8,14,6,11,3,4,9,7,2,13,12,0,5,10},
        {3,13,4,7,15,2,8,14,12,0,1,10,6,9,11,5},
        {0,14,7,11,10,4,13,1,5,8,12,6,9,3,2,15},
        {13,8,10,1,3,15,4,2,11,6,7,12,0,5,14,9}
    },
    // S3
    {
        {10,0,9,14,6,3,15,5,1,13,12,7,11,4,2,8},
        {13,7,0,9,3,4,6,10,2,8,5,14,12,11,15,1},
        {13,6,4,9,8,15,3,0,11,1,2,12,5,10,14,7},
        {1,10,13,0,6,9,8,7,4,15,14,3,11,5,2,12}
    },
    // S4
    {
        {7,13,14,3,0,6,9,10,1,2,8,5,11,12,4,15},
        {13,8,11,5,6,15,0,3,4,7,2,12,1,10,14,9},
        {10,6,9,0,12,11,7,13,15,1,3,14,5,2,8,4},
        {3,15,0,6,10,1,13,8,9,4,5,11,12,7,2,14}
    },
    // S5
    {
        {2,12,4,1,7,10,11,6,8,5,3,15,13,0,14,9},
        {14,11,2,12,4,7,13,1,5,0,15,10,3,9,8,6},
        {4,2,1,11,10,13,7,8,15,9,12,5,6,3,0,14},
        {11,8,12,7,1,14,2,13,6,15,0,9,10,4,5,3}
    },
    // S6
    {
        {12,1,10,15,9,2,6,8,0,13,3,4,14,7,5,11},
        {10,15,4,2,7,12,9,5,6,1,13,14,0,11,3,8},
        {9,14,15,5,2,8,12,3,7,0,4,10,1,13,11,6},
        {4,3,2,12,9,5,15,10,11,14,1,7,6,0,8,13}
    },
    // S7
    {
        {4,11,2,14,15,0,8,13,3,12,9,7,5,10,6,1},
        {13,0,11,7,4,9,1,10,14,3,5,12,2,15,8,6},
        {1,4,11,13,12,3,7,14,10,15,6,8,0,5,9,2},
        {6,11,13,8,1,4,10,7,9,5,0,15,14,2,3,12}
    },
    // S8
    {
        {13,2,8,4,6,15,11,1,10,9,3,14,5,0,12,7},
        {1,15,13,8,10,3,7,4,12,5,6,11,0,14,9,2},
        {7,11,4,1,9,12,14,2,0,6,10,13,15,3,5,8},
        {2,1,14,7,4,10,8,13,15,12,9,0,3,5,6,11}
    }
};

int SHIFT[16] = {
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2
};

///////////////////////////////////////////////////////////////////////////////
// Helper functions (bit handling)
///////////////////////////////////////////////////////////////////////////////

std::vector<int> bytesToBits(const std::vector<int>& src, int bitCount)
{
    std::vector<int> dst(bitCount);
    int pos = 0;
    for (int i = 0; i < (int)src.size(); ++i)
    {
        int val = src[i];
        for (int b = 7; b >= 0; --b)
        {
            dst[pos++] = (val >> b) & 1;
        }
    }
    return dst;
}

std::vector<int> bitsToBytes(const std::vector<int>& src)
{
    int byteCnt = src.size() / 8;
    std::vector<int> dst(byteCnt);
    int pos = 0;
    for (int i = 0; i < byteCnt; ++i)
    {
        int val = 0;
        for (int b = 0; b < 8; ++b)
        {
            val = (val << 1) | src[pos++];
        }
        dst[i] = val;
    }
    return dst;
}

void permute(const std::vector<int>& in, std::vector<int>& out, const int* table, int outSize)
{
    for (int i = 0; i < outSize; ++i)
        out[i] = in[table[i] - 1];
}

void rotateLeft(std::vector<int>& arr, int steps)
{
    int n = arr.size();
    while (steps--)
    {
        int first = arr[0];
        int i = 0;
        while (i < n - 1)
        {
            arr[i] = arr[i + 1];
            ++i;
        }
        arr[n - 1] = first;
    }
}

std::vector<int> xorVec(const std::vector<int>& a, const std::vector<int>& b)
{
    std::vector<int> out(a.size());
    int i = 0;
    while (i < (int)a.size())
    {
        out[i] = a[i] ^ b[i];
        ++i;
    }
    return out;
}

///////////////////////////////////////////////////////////////////////////////
// DES core class
///////////////////////////////////////////////////////////////////////////////

class DES
{
public:
    DES(const std::vector<int>& keyBytes)
    {
        keyBits = bytesToBits(keyBytes, 64);
        generateRoundKeys();
    }

    std::vector<int> encryptBlock(const std::vector<int>& plain)
    {
        std::vector<int> blockBits = bytesToBits(plain, 64);
        std::vector<int> ipBits(64);
        permute(blockBits, ipBits, IP, 64);

        std::vector<int> left(ipBits.begin(), ipBits.begin() + 32);
        std::vector<int> right(ipBits.begin() + 32, ipBits.end());

        int round = 0;
        while (round < 16)
        {
            std::vector<int> fOut = fFunction(right, roundKeys[round]);
            std::vector<int> newRight = xorVec(left, fOut);
            left = right;
            right = newRight;
            ++round;
        }

        // note the swap after the last round
        std::vector<int> preoutput;
        preoutput.reserve(64);
        preoutput.insert(preoutput.end(), right.begin(), right.end());
        preoutput.insert(preoutput.end(), left.begin(), left.end());

        std::vector<int> fpBits(64);
        permute(preoutput, fpBits, FP, 64);
        return bitsToBytes(fpBits);
    }

    std::vector<int> decryptBlock(const std::vector<int>& cipher)
    {
        std::vector<int> blockBits = bytesToBits(cipher, 64);
        std::vector<int> ipBits(64);
        permute(blockBits, ipBits, IP, 64);

        std::vector<int> left(ipBits.begin(), ipBits.begin() + 32);
        std::vector<int> right(ipBits.begin() + 32, ipBits.end());

        int round = 15;
        while (round >= 0)
        {
            std::vector<int> fOut = fFunction(right, roundKeys[round]);
            std::vector<int> newRight = xorVec(left, fOut);
            left = right;
            right = newRight;
            --round;
        }

        std::vector<int> preoutput;
        preoutput.reserve(64);
        preoutput.insert(preoutput.end(), right.begin(), right.end());
        preoutput.insert(preoutput.end(), left.begin(), left.end());

        std::vector<int> fpBits(64);
        permute(preoutput, fpBits, FP, 64);
        return bitsToBytes(fpBits);
    }

private:
    std::vector<int> keyBits;                     // 64 bits
    std::vector< std::vector<int> > roundKeys;    // 16 × 48 bits

    void generateRoundKeys()
    {
        std::vector<int> permutedKey(56);
        permute(keyBits, permutedKey, PC1, 56);

        std::vector<int> C(permutedKey.begin(), permutedKey.begin() + 28);
        std::vector<int> D(permutedKey.begin() + 28, permutedKey.end());

        roundKeys.clear();
        roundKeys.reserve(16);
        int r = 0;
        while (r < 16)
        {
            rotateLeft(C, SHIFT[r]);
            rotateLeft(D, SHIFT[r]);

            std::vector<int> combined;
            combined.reserve(56);
            combined.insert(combined.end(), C.begin(), C.end());
            combined.insert(combined.end(), D.begin(), D.end());

            std::vector<int> subKey(48);
            permute(combined, subKey, PC2, 48);
            roundKeys.push_back(subKey);
            ++r;
        }
    }

    std::vector<int> fFunction(const std::vector<int>& halfBlock, const std::vector<int>& subKey)
    {
        std::vector<int> expanded(48);
        permute(halfBlock, expanded, E, 48);
        std::vector<int> xored = xorVec(expanded, subKey);

        std::vector<int> sOut(32);
        int outPos = 0;
        int i = 0;
        while (i < 8)
        {
            int row = (xored[i * 6] << 1) | xored[i * 6 + 5];
            int col = (xored[i * 6 + 1] << 3) |
                      (xored[i * 6 + 2] << 2) |
                      (xored[i * 6 + 3] << 1) |
                      xored[i * 6 + 4];
            int val = Sbox[i][row][col];
            int b = 3;
            while (b >= 0)
            {
                sOut[outPos++] = (val >> b) & 1;
                --b;
            }
            ++i;
        }

        std::vector<int> pOut(32);
        permute(sOut, pOut, P, 32);
        return pOut;
    }
};

///////////////////////////////////////////////////////////////////////////////
// Triple-DES wrapper (EDE3)
///////////////////////////////////////////////////////////////////////////////

class TripleDES
{
public:
    TripleDES(const std::vector<int>& k1,
              const std::vector<int>& k2,
              const std::vector<int>& k3)
        : d1(k1), d2(k2), d3(k3) {}

    std::vector<int> encrypt(const std::vector<int>& pt)
    {
        std::vector<int> step1 = d1.encryptBlock(pt);
        std::vector<int> step2 = d2.decryptBlock(step1);
        return d3.encryptBlock(step2);
    }

    std::vector<int> decrypt(const std::vector<int>& ct)
    {
        std::vector<int> step1 = d3.decryptBlock(ct);
        std::vector<int> step2 = d2.encryptBlock(step1);
        return d1.decryptBlock(step2);
    }

private:
    DES d1, d2, d3;
};

///////////////////////////////////////////////////////////////////////////////
// Main – deterministic test vector
///////////////////////////////////////////////////////////////////////////////

int main()
{
    // Plaintext: eight identical bytes (duplicate-heavy)
    std::vector<int> plain = {0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA};

    // Keys: all three keys identical and each byte duplicated
    std::vector<int> keyA = {0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11};
    std::vector<int> keyB = {0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11};
    std::vector<int> keyC = {0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11};

    TripleDES tdes(keyA, keyB, keyC);

    std::vector<int> cipher = tdes.encrypt(plain);
    std::vector<int> recovered = tdes.decrypt(cipher);

    // Output
    std::cout << "Plaintext : ";
    for (int i = 0; i < (int)plain.size(); ++i)
        std::cout << std::hex << std::setw(2) << std::setfill('0') << plain[i];
    std::cout << std::dec << "\n";

    std::cout << "Ciphertext: ";
    for (int i = 0; i < (int)cipher.size(); ++i)
        std::cout << std::hex << std::setw(2) << std::setfill('0') << cipher[i];
    std::cout << std::dec << "\n";

    std::cout << "Recovered : ";
    for (int i = 0; i < (int)recovered.size(); ++i)
        std::cout << std::hex << std::setw(2) << std::setfill('0') << recovered[i];
    std::cout << std::dec << "\n";

    return 0;
}
