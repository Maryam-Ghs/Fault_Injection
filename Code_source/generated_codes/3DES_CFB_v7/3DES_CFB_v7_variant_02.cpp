#include <iostream>
#include <iomanip>

/* LLM input variant 2: small-diverse */

// simple block "DES" – just XOR with key and rotate bits (purely illustrative)
void desBlock(int* src, int* dst, int* key) {
    int i = 0;
    while (i < 8) {
        // fused expression: rotate left 3 bits, then XOR with key byte
        dst[i] = ((src[i] << 3) | (src[i] >> 5)) ^ key[i];
        ++i;
    }
}

// 3‑DES EDE mode (encrypt‑decrypt‑encrypt) using the toy block above
void tripleDes(int* src, int* dst, int* k1, int* k2, int* k3) {
    int temp[8];
    int i = 0;
    while (i < 8) { temp[i] = src[i]; ++i; }          // copy src → temp
    desBlock(temp, dst, k1);                         // encrypt with k1
    i = 0;
    while (i < 8) { temp[i] = dst[i]; ++i; }          // temp ← result
    desBlock(temp, dst, k2);                         // "decrypt" with k2 (same op)
    i = 0;
    while (i < 8) { temp[i] = dst[i]; ++i; }          // temp ← result
    desBlock(temp, dst, k3);                         // encrypt with k3
}

// CFB‑8 encryption (one byte per feedback)
void cfbEncrypt(int* plain, int* cipher, int len, int* iv,
                int* k1, int* k2, int* k3) {
    int shiftReg[8];
    int i = 0;
    while (i < 8) { shiftReg[i] = iv[i]; ++i; }      // init shift register with IV

    i = 0;
    while (i < len) {
        int ksBlock[8];
        tripleDes(shiftReg, ksBlock, k1, k2, k3);    // keystream block

        // take first keystream byte, fuse XOR and assignment
        cipher[i] = plain[i] ^ ksBlock[0];

        // shift register left by one byte and append new ciphertext byte
        int j = 0;
        while (j < 7) { shiftReg[j] = shiftReg[j + 1]; ++j; }
        shiftReg[7] = cipher[i];

        ++i;
    }
}

// helper to print byte arrays in hex
void printHex(int* data, int len) {
    int i = 0;
    while (i < len) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (data[i] & 0xFF);
        if (i + 1 < len) std::cout << ' ';
        ++i;
    }
    std::cout << std::dec << std::endl;
}

// ------------------------------------------------------------------
// version #7 – edge‑case heavy test harness (modified input variant)
int main() {
    // diverse small lengths
    int sizes[6] = {2, 3, 5, 7, 10, 12};

    // static IV (8 bytes) with a different pattern
    int iv[8];
    int idx = 0;
    while (idx < 8) { iv[idx] = ((idx * 17) + 5) & 0xFF; ++idx; }

    // three keys, each 8 bytes, new simple pattern
    int keyA[8], keyB[8], keyC[8];
    idx = 0;
    while (idx < 8) {
        keyA[idx] = ((idx * 5) + 9) & 0xFF;
        keyB[idx] = ((idx * 9) + 2) & 0xFF;
        keyC[idx] = ((idx * 13) + 4) & 0xFF;
        ++idx;
    }

    // process each test size
    idx = 0;
    while (idx < 6) {
        int curLen = sizes[idx];
        int* plain = new int[curLen];
        int* cipher = new int[curLen];

        // generate plain data: patterned sequence 0xF0,0x0F,0xAA repeating
        int i = 0;
        while (i < curLen) {
            if (i % 3 == 0) plain[i] = 0xF0;
            else if (i % 3 == 1) plain[i] = 0x0F;
            else plain[i] = 0xAA;
            ++i;
        }

        cfbEncrypt(plain, cipher, curLen, iv, keyA, keyB, keyC);

        std::cout << "Len=" << curLen << "\nPlain : ";
        printHex(plain, curLen);
        std::cout << "Cipher: ";
        printHex(cipher, curLen);
        std::cout << "--------------------------\n";

        delete[] plain;
        delete[] cipher;
        ++idx;
    }
    return 0;
}
