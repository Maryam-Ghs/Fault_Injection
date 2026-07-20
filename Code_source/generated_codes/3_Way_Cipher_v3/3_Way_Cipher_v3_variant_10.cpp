#include <iostream>
#include <iomanip>

/* LLM input variant 10: large-safe-stress */

class ThreeWayCipher {
public:
    // Encrypt a block using three key blocks
    void encryptBlock(const int* plain, int* crypt,
                      const int* keyA, const int* keyB, const int* keyC,
                      int blockSize,
                      int sizeA, int sizeB, int sizeC) {
        int idx = 0;
        while (idx < blockSize) {
            // Step‑1: add keyA
            int step1 = plain[idx] + keyA[idx % sizeA];
            // Step‑2: XOR with keyB
            int step2 = step1 ^ keyB[idx % sizeB];
            // Step‑3: subtract keyC then add keyA again (reordered arithmetic)
            int step3 = (step2 - keyC[idx % sizeC]) + keyA[idx % sizeA];
            crypt[idx] = step3;
            idx = idx + 1;               // explicit increment (while‑loop style)
        }
    }

    // Decrypt a block using the same three key blocks
    void decryptBlock(const int* crypt, int* plain,
                      const int* keyA, const int* keyB, const int* keyC,
                      int blockSize,
                      int sizeA, int sizeB, int sizeC) {
        int pos = 0;
        while (pos < blockSize) {
            // Reverse Step‑3
            int rev3 = crypt[pos] - keyA[pos % sizeA] + keyC[pos % sizeC];
            // Reverse Step‑2
            int rev2 = rev3 ^ keyB[pos % sizeB];
            // Reverse Step‑1
            int rev1 = rev2 - keyA[pos % sizeA];
            plain[pos] = rev1;
            pos = pos + 1;               // explicit increment
        }
    }
};

int main() {
    // -----------------------------------------------------------------
    // 1. Prepare larger predefined arrays (stack allocation)
    // -----------------------------------------------------------------
    const int BLOCK_SIZE = 1024;
    const int KEY_A_SIZE = 256;
    const int KEY_B_SIZE = 128;
    const int KEY_C_SIZE = 64;

    int plainBlock[BLOCK_SIZE];
    int keyAlpha[KEY_A_SIZE];
    int keyBeta[KEY_B_SIZE];
    int keyGamma[KEY_C_SIZE];

    // Deterministic initialization
    for (int i = 0; i < BLOCK_SIZE; ++i) {
        plainBlock[i] = (i * 31 + 7) & 0xFF; // simple pattern within byte range
    }
    for (int i = 0; i < KEY_A_SIZE; ++i) {
        keyAlpha[i] = (i * 13 + 5) & 0xFF;
    }
    for (int i = 0; i < KEY_B_SIZE; ++i) {
        keyBeta[i] = (i * 17 + 3) & 0xFF;
    }
    for (int i = 0; i < KEY_C_SIZE; ++i) {
        keyGamma[i] = (i * 19 + 11) & 0xFF;
    }

    // Buffers for ciphertext and recovered plaintext
    int cipherBlock[BLOCK_SIZE];
    int recovered[BLOCK_SIZE];

    // -----------------------------------------------------------------
    // 2. Create cipher object and run encryption
    // -----------------------------------------------------------------
    ThreeWayCipher cipher;
    cipher.encryptBlock(plainBlock, cipherBlock,
                        keyAlpha, keyBeta, keyGamma,
                        BLOCK_SIZE,
                        KEY_A_SIZE, KEY_B_SIZE, KEY_C_SIZE);

    // -----------------------------------------------------------------
    // 3. Output results (first 16 values for brevity)
    // -----------------------------------------------------------------
    const int DISPLAY_COUNT = 16;

    std::cout << "Plaintext (decimal, first " << DISPLAY_COUNT << "): ";
    for (int i = 0; i < DISPLAY_COUNT; ++i) {
        std::cout << plainBlock[i] << ' ';
    }
    std::cout << "\nPlaintext (hex, first " << DISPLAY_COUNT << "): ";
    for (int i = 0; i < DISPLAY_COUNT; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (plainBlock[i] & 0xFF) << ' ';
    }

    std::cout << std::dec << "\n\nCiphertext (decimal, first " << DISPLAY_COUNT << "): ";
    for (int i = 0; i < DISPLAY_COUNT; ++i) {
        std::cout << cipherBlock[i] << ' ';
    }
    std::cout << "\nCiphertext (hex, first " << DISPLAY_COUNT << "): ";
    for (int i = 0; i < DISPLAY_COUNT; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (cipherBlock[i] & 0xFF) << ' ';
    }
    std::cout << std::dec << "\n";

    // -----------------------------------------------------------------
    // 4. Decrypt and verify
    // -----------------------------------------------------------------
    cipher.decryptBlock(cipherBlock, recovered,
                        keyAlpha, keyBeta, keyGamma,
                        BLOCK_SIZE,
                        KEY_A_SIZE, KEY_B_SIZE, KEY_C_SIZE);

    std::cout << "\nRecovered (decimal, first " << DISPLAY_COUNT << "): ";
    for (int i = 0; i < DISPLAY_COUNT; ++i) {
        std::cout << recovered[i] << ' ';
    }
    std::cout << "\nRecovered (hex, first " << DISPLAY_COUNT << "): ";
    for (int i = 0; i < DISPLAY_COUNT; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (recovered[i] & 0xFF) << ' ';
    }
    std::cout << std::dec << "\n";

    // Simple verification flag
    int ok = 1;
    int idx = 0;
    while (idx < BLOCK_SIZE) {
        ok = ok * (plainBlock[idx] == recovered[idx]);
        idx = idx + 1;
    }
    std::cout << "\nVerification: " << (ok ? "SUCCESS" : "FAILURE") << "\n";

    return 0;
}
