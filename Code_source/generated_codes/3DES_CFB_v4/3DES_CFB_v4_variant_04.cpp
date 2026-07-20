#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <ctime>

/* LLM input variant 4: signed-extremes */

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

        // deterministic signed values mixing negative, zero, and positive
        int presetA[8] = { -128, -64, 0, 1, 64, 127, -1, 32 };
        int presetB[8] = { 0, -32, 45, -45, 100, -100, 7, -7 };
        int presetC[8] = { 127, -127, 0, -2, 2, -50, 50, -3 };
        int presetIV[8] = { -5, 0, 5, -10, 10, -15, 15, 0 };

        for (int i = 0; i < blkSize; ++i) {
            keyA[i] = presetA[i];
            keyB[i] = presetB[i];
            keyC[i] = presetC[i];
            initVector[i] = presetIV[i];
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
    // deterministic signed plaintext mixing negative, zero, and positive values
    const int totalBytes = 256;
    int* plainData = new int[totalBytes];
    int pattern[8] = { -50, 0, 50, -1, 1, -100, 100, 0 };
    for (int idx = 0; idx < totalBytes; ++idx) {
        plainData[idx] = pattern[idx % 8];
    }

    // allocate buffer for ciphertext (same length)
    int* cipherData = new int[totalBytes];

    // Perform Triple‑DES CFB encryption
    TripleDESCFB encryptor;
    encryptor.cfbEncrypt(plainData, totalBytes, cipherData);

    // Print plaintext and ciphertext as hex strings
    std::printf("Plaintext :\n");
    for (int idx = 0; idx < totalBytes; ++idx) {
        std::printf("%02X ", plainData[idx] & 0xFF);
        if (((idx + 1) % 16) == 0)
            std::printf("\n");
    }

    std::printf("\nCiphertext (3DES‑CFB) :\n");
    for (int idx = 0; idx < totalBytes; ++idx) {
        std::printf("%02X ", cipherData[idx] & 0xFF);
        if (((idx + 1) % 16) == 0)
            std::printf("\n");
    }

    // Clean up heap memory
    delete[] plainData;
    delete[] cipherData;

    return 0;
}
