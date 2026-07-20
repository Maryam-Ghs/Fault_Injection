#include <iostream>
#include <cstdio>
#include <cstdlib>

/* LLM input variant 9: medium-deterministic-random */

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

        // deterministic fill for keys and IV
        int idx = 0;
        while (idx < blkSize)
        {
            keyA[idx] = (idx * 37 + 1) % 256;
            idx = idx + 1;
        }
        idx = 0;
        while (idx < blkSize)
        {
            keyB[idx] = (idx * 37 + 2) % 256;
            idx = idx + 1;
        }
        idx = 0;
        while (idx < blkSize)
        {
            keyC[idx] = (idx * 37 + 3) % 256;
            idx = idx + 1;
        }
        idx = 0;
        while (idx < blkSize)
        {
            initVector[idx] = (idx * 37 + 4) % 256;
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
    // -----------------------------------------------------------------
    // Create a deterministic plaintext (e.g., 240 bytes)
    // -----------------------------------------------------------------
    int totalBytes = 240;
    int* plainData = new int[totalBytes];
    int idx = 0;
    while (idx < totalBytes)
    {
        plainData[idx] = (idx * 53 + 7) % 256;
        idx = idx + 1;
    }

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
    idx = 0;
    while (idx < totalBytes)
    {
        std::printf("%02X ", plainData[idx]);
        // line break every 16 bytes for readability
        if (((idx + 1) % 16) == 0)
            std::printf("\n");
        idx = idx + 1;
    }

    std::printf("\nCiphertext (3DES‑CFB) :\n");
    idx = 0;
    while (idx < totalBytes)
    {
        std::printf("%02X ", cipherData[idx]);
        if (((idx + 1) % 16) == 0)
            std::printf("\n");
        idx = idx + 1;
    }

    // -----------------------------------------------------------------
    // Clean up heap memory
    // -----------------------------------------------------------------
    delete[] plainData;
    delete[] cipherData;

    return 0;
}
