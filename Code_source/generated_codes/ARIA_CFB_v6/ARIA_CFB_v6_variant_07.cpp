#include <iostream>
#include <vector>
#include <iomanip>

/* LLM input variant 7: reverse-adversarial */
int main() {
    // ------------------------------------------------------------
    // ARIA-CFB (Version #6) – reverse‑ordered adversarial test.
    // ------------------------------------------------------------

    int BLOCK = 2;          // small block to increase iteration count
    int KEYLEN = 2;
    int ROUNDS = 4;         // more rounds to increase work per byte

    // ----------------------------------------------------------------
    // 1️⃣  Keys in descending order (0xFF, 0xFE, ...)
    // ----------------------------------------------------------------
    std::vector< std::vector<int> > keyBank;

    std::vector<int> descKey(KEYLEN);
    int i = 0;
    while (i < KEYLEN) {
        descKey[i] = 0xFF - i;   // 0xFF, 0xFE
        i = i + 1;
    }
    keyBank.push_back(descKey);

    // ----------------------------------------------------------------
    // 2️⃣  Plaintext: length 31 (reverse‑ordered byte values)
    // ----------------------------------------------------------------
    std::vector< std::vector<int> > plainBank;

    std::vector<int> revPlain(31);
    i = 0;
    while (i < 31) {
        revPlain[i] = 30 - i;   // 30,29,...,0
        i = i + 1;
    }
    plainBank.push_back(revPlain);

    // ----------------------------------------------------------------
    // 3️⃣  Fixed IV (also descending)
    // ----------------------------------------------------------------
    std::vector<int> ivFixed(BLOCK);
    i = 0;
    while (i < BLOCK) {
        ivFixed[i] = 0xAA - i;   // 0xAA, 0xA9
        i = i + 1;
    }

    // ----------------------------------------------------------------
    // 4️⃣  Iterate over key and plaintext
    // ----------------------------------------------------------------
    int keyIdx = 0;
    while (keyIdx < (int)keyBank.size()) {
        std::vector<int> curKey = keyBank[keyIdx];

        // round keys
        std::vector< std::vector<int> > roundKeySet;
        int r = 0;
        while (r < ROUNDS) {
            std::vector<int> rk(KEYLEN);
            int j = 0;
            while (j < KEYLEN) {
                rk[j] = curKey[j] ^ (r * 0x1F);
                j = j + 1;
            }
            roundKeySet.push_back(rk);
            r = r + 1;
        }

        int ptIdx = 0;
        while (ptIdx < (int)plainBank.size()) {
            std::vector<int> curPlain = plainBank[ptIdx];
            std::vector<int> feedback = ivFixed;
            std::vector<int> cipherText;

            int pos = 0;
            while (pos < (int)curPlain.size()) {
                std::vector<int> encBlock = feedback;
                int round = 0;
                while (round < ROUNDS) {
                    std::vector<int> curRoundKey = roundKeySet[round];
                    int byte = 0;
                    while (byte < BLOCK) {
                        encBlock[byte] ^= curRoundKey[byte];
                        byte = byte + 1;
                    }
                    round = round + 1;
                }

                int remain = (int)curPlain.size() - pos;
                int chunk = BLOCK;
                if (remain < BLOCK) chunk = remain;

                int k = 0;
                while (k < chunk) {
                    int ctByte = curPlain[pos + k] ^ encBlock[k];
                    cipherText.push_back(ctByte);

                    int shift = 0;
                    while (shift < BLOCK - 1) {
                        feedback[shift] = feedback[shift + 1];
                        shift = shift + 1;
                    }
                    feedback[BLOCK - 1] = ctByte;
                    k = k + 1;
                }
                pos = pos + chunk;
            }

            std::cout << "KeyIdx=" << keyIdx
                      << " PlainIdx=" << ptIdx
                      << " CipherHex=";
            int out = 0;
            while (out < (int)cipherText.size()) {
                std::cout << std::hex << std::setw(2) << std::setfill('0')
                          << (cipherText[out] & 0xFF);
                out = out + 1;
            }
            std::cout << std::dec << std::endl;

            ptIdx = ptIdx + 1;
        }

        keyIdx = keyIdx + 1;
    }

    return 0;
}
