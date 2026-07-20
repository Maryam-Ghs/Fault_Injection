#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <ctime>

/* LLM input variant 1: minimal-boundary */

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

        // deterministic key and IV values (0‑255)
        for (int i = 0; i < blkSize; ++i) {
            keyA[i] = i;               // 0..7
            keyB[i] = (i + 1) & 0xFF;  // 1..8
            keyC[i] = (i + 2) & 0xFF;  // 2..9
            initVector[i] = (i + 3) & 0xFF; // 3..10
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
        for (int i = 0; i < blkSize; ++i)
            shiftReg[i] = initVector[i];

        int blockPos = 0;
        while (blockPos < plainLen)
        {
            // 1) encrypt the current shift register
            int keystream[8];
            tripleDesBlock(shiftReg, keystream);

            // 2) XOR keystream with current plaintext block
            for (int j = 0; j < blkSize; ++j)
                cipher[blockPos + j] = keystream[j] ^ plain[blockPos + j];

            // 3) feed the ciphertext block back into the shift register
            for (int k = 0; k < blkSize; ++k)
                shiftReg[k] = cipher[blockPos + k];

            blockPos = blockPos + blkSize;
        }
    }
};

int main()
{
    std::srand(static_cast<unsigned int>(std::time(0)));

    // -----------------------------------------------------------------
    // Minimal plaintext (one block, 8 bytes)
    // -----------------------------------------------------------------
    int totalBytes = 8;
    int* plainData = new int[totalBytes];
    for (int idx = 0; idx < totalBytes; ++idx)
        plainData[idx] = idx; // deterministic values 0..7

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
    for (int idx = 0; idx < totalBytes; ++idx) {
        std::printf("%02X ", plainData[idx]);
        if ((idx + 1) % 16 == 0) std::printf("\n");
    }

    std::printf("\nCiphertext (3DES‑CFB) :\n");
    for (int idx = 0; idx < totalBytes; ++idx) {
        std::printf("%02X ", cipherData[idx]);
        if ((idx + 1) % 16 == 0) std::printf("\n");
    }

    // -----------------------------------------------------------------
    // Clean up heap memory
    // -----------------------------------------------------------------
    delete[] plainData;
    delete[] cipherData;

    return 0;
}
