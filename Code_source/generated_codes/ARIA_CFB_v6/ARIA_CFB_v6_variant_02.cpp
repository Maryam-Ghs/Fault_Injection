#include <iostream>
#include <vector>
#include <iomanip>

/* LLM input variant 2: small-diverse */
int main() {
    // ------------------------------------------------------------
    // ARIA-CFB (Version #6) – completely self‑contained, verbose,
    // step‑by‑step, using only int and std::vector.
    // ------------------------------------------------------------

    // ----------------------------------------------------------------
    // Parameters (all plain int, no const, no unsigned, no double)
    // ----------------------------------------------------------------
    int BLOCK = 16;
    int KEYLEN = 16;
    int ROUNDS = 2;          // small number of rounds for speed

    // ----------------------------------------------------------------
    // 1️⃣  Build a collection of edge‑case keys
    // ----------------------------------------------------------------
    std::vector< std::vector<int> > keyBank;

    // all‑zero key
    std::vector<int> keyZero(KEYLEN, 0);
    keyBank.push_back(keyZero);

    // all‑ones key (0xFF)
    std::vector<int> keyOnes(KEYLEN, 255);
    keyBank.push_back(keyOnes);

    // incremental pattern key
    std::vector<int> keyPattern(KEYLEN);
    int i = 0;
    while (i < KEYLEN) {
        keyPattern[i] = (i * 17) & 0xFF;
        i = i + 1;
    }
    keyBank.push_back(keyPattern);

    // alternating 0xAA / 0x55 pattern
    std::vector<int> keyAlt(KEYLEN);
    i = 0;
    while (i < KEYLEN) {
        keyAlt[i] = (i % 2 == 0) ? 0xAA : 0x55;
        i = i + 1;
    }
    keyBank.push_back(keyAlt);

    // ----------------------------------------------------------------
    // 2️⃣  Build a collection of edge‑case plaintexts
    // ----------------------------------------------------------------
    std::vector< std::vector<int> > plainBank;

    // empty plaintext
    plainBank.push_back(std::vector<int>());

    // single‑byte plaintext
    plainBank.push_back(std::vector<int>(1, 0xAB));

    // exactly one block (16 bytes)
    std::vector<int> oneBlock(BLOCK);
    i = 0;
    while (i < BLOCK) {
        oneBlock[i] = (i * 3) & 0xFF;
        i = i + 1;
    }
    plainBank.push_back(oneBlock);

    // non‑multiple length (20 bytes)
    std::vector<int> twentyBytes(20);
    i = 0;
    while (i < 20) {
        twentyBytes[i] = (i * 7) & 0xFF;
        i = i + 1;
    }
    plainBank.push_back(twentyBytes);

    // three blocks (48 bytes)
    std::vector<int> threeBlocks(BLOCK * 3);
    i = 0;
    while (i < BLOCK * 3) {
        threeBlocks[i] = (i * 5) & 0xFF;
        i = i + 1;
    }
    plainBank.push_back(threeBlocks);

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
                    int keystreamByte = encBlock[k];
                    int ctByte = ptByte ^ keystreamByte;
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
