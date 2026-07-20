#include <iostream>
#include <vector>
#include <iomanip>

/* LLM input variant 3: zeros-and-ones */
int main() {
    // ------------------------------------------------------------
    // ARIA-CFB (Version #6) – completely self‑contained, verbose,
    // step‑by‑step, using only int and std::vector.
    // ------------------------------------------------------------

    int BLOCK = 16;
    int KEYLEN = 16;
    int ROUNDS = 2;

    // ----------------------------------------------------------------
    // 1️⃣  Build a collection of edge‑case keys (zeros, ones, alternating)
    // ----------------------------------------------------------------
    std::vector< std::vector<int> > keyBank;

    // all‑zero key
    keyBank.push_back(std::vector<int>(KEYLEN, 0));

    // all‑ones key
    keyBank.push_back(std::vector<int>(KEYLEN, 255));

    // alternating 0/1 key
    std::vector<int> keyAlt(KEYLEN);
    int i = 0;
    while (i < KEYLEN) {
        keyAlt[i] = (i % 2 == 0) ? 0 : 1;
        i = i + 1;
    }
    keyBank.push_back(keyAlt);

    // ----------------------------------------------------------------
    // 2️⃣  Build a collection of edge‑case plaintexts
    // ----------------------------------------------------------------
    std::vector< std::vector<int> > plainBank;

    // empty plaintext
    plainBank.push_back(std::vector<int>());

    // 16‑byte all zeros
    plainBank.push_back(std::vector<int>(BLOCK, 0));

    // 16‑byte all ones (0xFF)
    plainBank.push_back(std::vector<int>(BLOCK, 255));

    // single zero byte
    plainBank.push_back(std::vector<int>(1, 0));

    // single one byte
    plainBank.push_back(std::vector<int>(1, 1));

    // ----------------------------------------------------------------
    // 3️⃣  Fixed IV (initial vector) for CFB mode
    // ----------------------------------------------------------------
    std::vector<int> ivFixed(BLOCK);
    i = 0;
    while (i < BLOCK) {
        ivFixed[i] = (i * 11) & 0xFF;
        i = i + 1;
    }

    // ----------------------------------------------------------------
    // 4️⃣  Iterate over every key
    // ----------------------------------------------------------------
    int keyIdx = 0;
    while (keyIdx < (int)keyBank.size()) {
        std::vector<int> curKey = keyBank[keyIdx];

        // generate round keys
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

        // ----------------------------------------------------------------
        // 5️⃣  Iterate over every plaintext
        // ----------------------------------------------------------------
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
                    int ptByte = curPlain[pos + k];
                    int ctByte = ptByte ^ encBlock[k];
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
