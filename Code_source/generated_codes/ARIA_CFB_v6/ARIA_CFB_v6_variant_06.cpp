#include <iostream>
#include <vector>
#include <iomanip>

/* LLM input variant 6: ordered-structured */
int main() {
    // ------------------------------------------------------------
    // ARIA-CFB (Version #6) – ordered and structured test data.
    // ------------------------------------------------------------

    int BLOCK = 16;
    int KEYLEN = 16;
    int ROUNDS = 3;

    // ----------------------------------------------------------------
    // 1️⃣  Ordered keys (ascending byte values)
    // ----------------------------------------------------------------
    std::vector< std::vector<int> > keyBank;

    // key 0..15
    std::vector<int> ascKey(KEYLEN);
    int i = 0;
    while (i < KEYLEN) {
        ascKey[i] = i;
        i = i + 1;
    }
    keyBank.push_back(ascKey);

    // key 16..31 (wrapped modulo 256)
    std::vector<int> ascKey2(KEYLEN);
    i = 0;
    while (i < KEYLEN) {
        ascKey2[i] = (i + KEYLEN) & 0xFF;
        i = i + 1;
    }
    keyBank.push_back(ascKey2);

    // ----------------------------------------------------------------
    // 2️⃣  Structured plaintexts: lengths 0,1,2,...,32 (multiples of BLOCK where possible)
    // ----------------------------------------------------------------
    std::vector< std::vector<int> > plainBank;

    // length 0
    plainBank.push_back(std::vector<int>());

    // lengths 1 through 15 (incremental)
    i = 1;
    while (i <= 15) {
        std::vector<int> p(i);
        int j = 0;
        while (j < i) {
            p[j] = (j * 7) & 0xFF;
            j = j + 1;
        }
        plainBank.push_back(p);
        i = i + 1;
    }

    // length 16 (exact block)
    std::vector<int> block16(BLOCK);
    i = 0;
    while (i < BLOCK) {
        block16[i] = (i * 5) & 0xFF;
        i = i + 1;
    }
    plainBank.push_back(block16);

    // length 24 (1.5 blocks)
    std::vector<int> len24(24);
    i = 0;
    while (i < 24) {
        len24[i] = (i * 9) & 0xFF;
        i = i + 1;
    }
    plainBank.push_back(len24);

    // length 32 (two blocks)
    std::vector<int> len32(BLOCK * 2);
    i = 0;
    while (i < BLOCK * 2) {
        len32[i] = (i * 11) & 0xFF;
        i = i + 1;
    }
    plainBank.push_back(len32);

    // ----------------------------------------------------------------
    // 3️⃣  Fixed IV
    // ----------------------------------------------------------------
    std::vector<int> ivFixed(BLOCK);
    i = 0;
    while (i < BLOCK) {
        ivFixed[i] = (i * 13) & 0xFF;
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
