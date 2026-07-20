#include <iostream>
#include <iomanip>

/* LLM input variant 4: signed-extremes */

// version #10
#define KEY_XOR 0xAA
#define KEY_ADD 7
#define ROT_BITS 3

// generate a heap‑allocated int array containing the ASCII codes of a string
int* buildArray(const char* src, int& outLen) {
    outLen = 0;
    while (src[outLen] != '\0') ++outLen;
    int* buf = new int[outLen];
    int idx = 0;
    while (idx < outLen) {
        buf[idx] = static_cast<int>(src[idx]);
        ++idx;
    }
    return buf;
}

// 3‑Way encryption: XOR → rotate left → add
void encrypt3Way(int* data, int len) {
    int i = 0;
    while (i < len) {
        // fused expression: XOR then rotate then add, all masked to 8 bits
        data[i] = (( (data[i] ^ KEY_XOR) << ROT_BITS) |
                   ((data[i] ^ KEY_XOR) >> (8 - ROT_BITS)) ) & 0xFF;
        data[i] = (data[i] + KEY_ADD) & 0xFF;
        ++i;
    }
}

// 3‑Way decryption: subtract → rotate right → XOR
void decrypt3Way(int* data, int len) {
    int i = 0;
    while (i < len) {
        data[i] = (data[i] - KEY_ADD) & 0xFF;
        data[i] = ((data[i] >> ROT_BITS) |
                   (data[i] << (8 - ROT_BITS))) & 0xFF;
        data[i] = data[i] ^ KEY_XOR;
        ++i;
    }
}

// print an int array as hex values
void showHex(const int* arr, int len) {
    int i = 0;
    while (i < len) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << (arr[i] & 0xFF);
        if (i + 1 < len) std::cout << ' ';
        ++i;
    }
    std::cout << std::dec << std::endl;
}

// print an int array as characters (non‑printable shown as '.')
void showChars(const int* arr, int len) {
    int i = 0;
    while (i < len) {
        char ch = static_cast<char>(arr[i] & 0xFF);
        if (ch < 32 || ch > 126) ch = '.';
        std::cout << ch;
        ++i;
    }
    std::cout << std::endl;
}

// driver – creates signed‑extreme inputs, encrypts, decrypts and prints them
int main() {
    // custom test cases mixing zero and positive extremes (no negatives to avoid UB)
    const int caseLengths[] = {0, 1, 5};
    const int caseData0[] = {};                                 // empty
    const int caseData1[] = {0};                                // single zero
    const int caseData2[] = {0, 1, 127, 128, 255};              // extremes within 0‑255

    const int* predefined[] = {caseData0, caseData1, caseData2};

    int caseIdx = 0;
    while (caseIdx < 3) {
        int txtLen = caseLengths[caseIdx];
        int* txtArr = nullptr;
        if (txtLen > 0) {
            txtArr = new int[txtLen];
            for (int i = 0; i < txtLen; ++i) {
                txtArr[i] = predefined[caseIdx][i];
            }
        }

        std::cout << "Case " << caseIdx + 1 << " – original: ";
        showChars(txtArr, txtLen);
        std::cout << "          – length: " << txtLen << std::endl;

        // encrypt in‑place
        encrypt3Way(txtArr, txtLen);
        std::cout << "          – encrypted (hex): ";
        showHex(txtArr, txtLen);

        // decrypt back
        decrypt3Way(txtArr, txtLen);
        std::cout << "          – decrypted: ";
        showChars(txtArr, txtLen);
        std::cout << std::endl;

        delete[] txtArr;
        ++caseIdx;
    }
    return 0;
}
