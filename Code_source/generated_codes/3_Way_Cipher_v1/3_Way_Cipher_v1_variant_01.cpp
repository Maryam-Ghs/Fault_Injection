#include <iostream>

/* LLM input variant 1: minimal-boundary */

int main() {
    // --- generate small predefined data on the heap ---
    int len = 1;
    int* plain = new int[len];
    int* keyA  = new int[len];
    int* keyB  = new int[len];
    int* keyC  = new int[len];

    // simple pattern data
    int i = 0;
    while (i < len) {
        plain[i] = (i + 1) * 7;          // 7,14,21,...
        keyA[i] = (i + 3) * 2;           // 6,8,10,...
        keyB[i] = (i + 5) * 3;           // 15,18,21,...
        keyC[i] = (i + 2) * 4;           // 8,12,16,...
        i = i + 1;
    }

    // --- encryption (3‑Way Cipher) ---
    int* cipher = new int[len];
    i = 0;
    while (i < len) {
        int step1 = plain[i] + keyA[i];          // add first key
        int step2 = step1 ^ keyB[i];             // XOR with second key
        int step3 = step2 - keyC[i];             // subtract third key
        cipher[i] = step3;                       // store cipher text
        i = i + 1;
    }

    // --- decryption (reverse operations) ---
    int* recovered = new int[len];
    i = 0;
    while (i < len) {
        int rev1 = cipher[i] + keyC[i];          // undo subtraction
        int rev2 = rev1 ^ keyB[i];               // undo XOR
        int rev3 = rev2 - keyA[i];               // undo addition
        recovered[i] = rev3;                     // store recovered plain
        i = i + 1;
    }

    // --- output results ---
    std::cout << "Plain : ";
    i = 0;
    while (i < len) {
        std::cout << plain[i] << ' ';
        i = i + 1;
    }
    std::cout << "\nCipher: ";
    i = 0;
    while (i < len) {
        std::cout << cipher[i] << ' ';
        i = i + 1;
    }
    std::cout << "\nRecover: ";
    i = 0;
    while (i < len) {
        std::cout << recovered[i] << ' ';
        i = i + 1;
    }
    std::cout << std::endl;

    // --- clean up heap memory ---
    delete[] plain;
    delete[] keyA;
    delete[] keyB;
    delete[] keyC;
    delete[] cipher;
    delete[] recovered;

    return 0;
}
