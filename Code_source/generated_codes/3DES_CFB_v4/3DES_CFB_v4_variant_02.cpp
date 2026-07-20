#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <ctime>

/* LLM input variant 2: small-diverse */

class TripleDESCFB
{
public:
    // keys and IV are stored on the heap
    int* keyA;
    int* keyB;
    int* keyC;
    int* initVector;

    // block size for CFB (8 bytes = 64 bits)
    int blkSize;

    TripleDESCFB()
    {
        blkSize = 8;                         // fixed block size
        keyA = new int[blkSize];
        keyB = new int[blkSize];
        keyC = new int[blkSize];
        initVector = new int[blkSize];

        // fill keys and IV with random byte values (0‑255)
        int idx = 0;
        while (idx < blkSize)
        {
            keyA[idx] = std::rand() % 256;
            idx = idx + 1;
        }
        idx = 0;
        while (idx < blkSize)
        {
            keyB[idx] = std::rand() % 256;
            idx = idx + 1;
        }
        idx = 0;
        while (idx < blkSize)
        {
            keyC[idx] = std::rand() % 256;
            idx = idx + 1;
        }
        idx = 0;
        while (idx < blkSize)
        {
            initVector[idx] = std::rand() % 256;
            idx = idx + 1;
        }
    }

    ~TripleDESCFB()
    {
        delete[] keyA;
        delete[] keyB;
        delete[] keyC;
        delete[] initVector;
    }

    // -----------------------------------------------------------------
    // Very simplified DES‑like round (just for demonstration)
    // -----------------------------------------------------------------
    void simpleDesEncrypt(int* src, int* dst, int* key)
    {
        int i = 0;
        while (i < blkSize)
        {
            int step1 = src[i] ^ key[i];          // XOR with key byte
            int step2 = step1 + 7;                // add constant
            int step3 = step2 & 0xFF;             // keep only low 8 bits
            dst[i] = step3;
            i = i + 1;
        }
    }

    void simpleDesDecrypt(int* src, int* dst, int* key)
    {
        int i = 0;
        while (i < blkSize)
        {
            int step1 = src[i] & 0xFF;            // ensure byte range
            int step2 = step1 - 7;                // reverse the addition
            int step3 = step2 ^ key[i];           // reverse XOR
            int step4 = step3 & 0xFF;             // keep low 8 bits
            dst[i] = step4;
            i = i + 1;
        }
    }

    // -----------------------------------------------------------------
    // Triple‑DES in E‑D‑E order (encrypt‑decrypt‑encrypt)
    // -----------------------------------------------------------------
    void tripleDesBlock(int* src, int* dst)
    {
        int tmp1[8];
        int tmp2[8];

        // first DES encrypt with keyA
        simpleDesEncrypt(src, tmp1, keyA);

        // second DES decrypt with keyB
        simpleDesDecrypt(tmp1, tmp2, keyB);

        // third DES encrypt with keyC
        simpleDesEncrypt(tmp2, dst, keyC);
    }

    // -----------------------------------------------------------------
    // CFB‑64 encryption (processes whole blocks)
    // -----------------------------------------------------------------
    void cfbEncrypt(int* plain, int plainLen, int* cipher)
    {
        // shift register holds the previous ciphertext block (or IV for first block)
        int shiftReg[8];
        int i = 0;
        while (i < blkSize)
        {
            shiftReg[i] = initVector[i];
            i = i + 1;
        }

        int blockPos = 0;
        while (blockPos < plainLen)
        {
            // 1) encrypt the current shift register
            int keystream[8];
            tripleDesBlock(shiftReg, keystream);

            // 2) XOR keystream with current plaintext block
            int j = 0;
            while (j < blkSize)
            {
                int xorStep = keystream[j] ^ plain[blockPos + j];
                cipher[blockPos + j] = xorStep;
                j = j + 1;
            }

            // 3) feed the ciphertext block back into the shift register
            int k = 0;
            while (k < blkSize)
            {
                shiftReg[k] = cipher[blockPos + k];
                k = k + 1;
            }

            blockPos = blockPos + blkSize;
        }
    }
};

int main()
{
    std::srand(static_cast<unsigned int>(std::time(0)));

    // -----------------------------------------------------------------
    // Small deterministic plaintext (32 bytes) with varied values
    // -----------------------------------------------------------------
    const int totalBytes = 32;
    int* plainData = new int[totalBytes];
    int predefined[32] = {
        0x00, 0x7F, 0x80, 0xFF,
        0x12, 0x34, 0x56, 0x78,
        0x9A, 0xBC, 0xDE, 0xF0,
        0x01, 0x23, 0x45, 0x67,
        0x89, 0xAB, 0xCD, 0xEF,
        0x10, 0x20, 0x30, 0x40,
        0x50, 0x60, 0x70, 0x80,
        0x90, 0xA0, 0xB0, 0xC0
    };
    for (int i = 0; i < totalBytes; ++i)
        plainData[i] = predefined[i];

    // -----------------------------------------------------------------
    // Allocate buffer for ciphertext (same length)
    // -----------------------------------------------------------------
    int* cipherData = new int[totalBytes];

    // -----------------------------------------------------------------
    // Perform Triple‑DES CFB encryption
    // -----------------------------------------------------------------
    TripleDESCFB encryptor;
    encryptor.cfbEncrypt(plainData, totalBytes, cipherData);

    // -----------------------------------------------------------------
    // Print plaintext and ciphertext as hex strings
    // -----------------------------------------------------------------
    std::printf("Plaintext :\n");
    for (int i = 0; i < totalBytes; ++i)
    {
        std::printf("%02X ", plainData[i]);
        if (((i + 1) % 16) == 0)
            std::printf("\n");
    }

    std::printf("\nCiphertext (3DES‑CFB) :\n");
    for (int i = 0; i < totalBytes; ++i)
    {
        std::printf("%02X ", cipherData[i]);
        if (((i + 1) % 16) == 0)
            std::printf("\n");
    }

    // -----------------------------------------------------------------
    // Clean up heap memory
    // -----------------------------------------------------------------
    delete[] plainData;
    delete[] cipherData;

    return 0;
}
