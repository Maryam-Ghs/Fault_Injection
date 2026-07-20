#include <iostream>
#include <vector>
#include <iomanip>

/* LLM input variant 5: duplicate-heavy */
int main() {
    // ------------------------------------------------------------
    // ARIA-CFB (Version #6) – duplicate‑heavy test data.
    // ------------------------------------------------------------

    int BLOCK = 16;
    int KEYLEN = 16;
    int ROUNDS = 2;

    // ----------------------------------------------------------------
    // 1️⃣  Duplicate keys (three identical zero keys, two identical pattern keys)
    // ----------------------------------------------------------------
    std::vector< std::vector<int> > keyBank;

    std::vector<int> zeroKey(KEYLEN, 0);
    keyBank.push_back(zeroKey);
    keyBank.push_back(zeroKey);
    keyBank.push_back(zeroKey);

    std::vector<int> patternKey(KEYLEN);
    int i = 0;
    while (i < KEYLEN) {
        patternKey[i] = (i * 17) & 0xFF;
        i = i + 1;
    }
    keyBank.push_back(patternKey);
    keyBank.push_back(patternKey);

    // ----------------------------------------------------------------
    // 2️⃣  Duplicate plaintexts (two empty, two single‑byte, two identical blocks)
    // ----------------------------------------------------------------
    std::vector< std::vector<int> > plainBank;

    // empty
    plainBank.push_back(std::vector<int>());
    plainBank.push_back(std::vector<int>());

    // single byte 0xAB
    plainBank.push_back(std::vector<int>(1, 0xAB));
    plainBank.push_back(std::vector<int>(1, 0xAB));

    // one full block (pattern)
    std::vector<int> oneBlock(BLOCK);
    i = 0;
    while (i < BLOCK) {
        oneBlock[i] = (i * 3) & 0xFF;
        i = i + 1;
    }
    plainBank.push_back(oneBlock);
    plainBank.push_back(oneBlock);

    // ----------------------------------------------------------------
    // 3️⃣  Fixed IV
    // ----------------------------------------------------------------
    std::vector<int> ivFixed(BLOCK);
    i = 0;
    while (i < BLOCK) {
        ivFixed[i] = (i * 11) & 0xFF;
        i = i + 1;
    }

    // ----------------------------------------------------------------
    // 4️⃣  Iterate over keys and plaintexts
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
