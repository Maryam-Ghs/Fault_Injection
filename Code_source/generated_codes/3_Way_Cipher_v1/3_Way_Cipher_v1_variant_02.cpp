#include <iostream>

/* LLM input variant 2: small-diverse */

int main() {
    // --- generate small predefined data on the heap ---
    int len = 6;
    int* plain = new int[len];
    int* keyA  = new int[len];
    int* keyB  = new int[len];
    int* keyC  = new int[len];

    // varied pattern data
    int i = 0;
    while (i < len) {
        // Plain text includes positive, zero, and negative numbers
        plain[i] = (i % 2 == 0) ? (i + 1) * 5 : -(i + 1) * 3;   // 5, -6, 15, -24, 35, -42
        // Key A alternates between a small positive and a negative constant
        keyA[i] = (i % 3 == 0) ? 7 : -4;                       // 7, -4, -4, 7, -4, -4
        // Key B grows linearly
        keyB[i] = (i + 2) * 2;                                 // 4, 6, 8,10,12,14
        // Key C switches sign each step
        keyC[i] = (i % 2 == 0) ? -1 : 3;                       // -1,3,-1,3,-1,3
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
