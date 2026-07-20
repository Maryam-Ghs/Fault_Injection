#include <iostream>
/* LLM input variant 7: reverse-adversarial */

int main() {
    // --- generate reverse‑ordered predefined data on the heap ---
    int len = 8;
    int* plain = new int[len];
    int* keyA  = new int[len];
    int* keyB  = new int[len];
    int* keyC  = new int[len];

    // reverse pattern data (worst‑case like ordering)
    int i = 0;
    while (i < len) {
        plain[i] = (len - i) * 7;          // 56,49,42,...
        keyA[i] = (len - i + 2) * 2;        // 20,18,16,...
        keyB[i] = (len - i + 4) * 3;        // 36,33,30,...
        keyC[i] = (len - i + 1) * 4;        // 36,32,28,...
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
