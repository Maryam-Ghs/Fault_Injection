#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

#define N 10000

/* LLM input variant 7: reverse-adversarial */

/*------------------------------------------------------------*/
/* Helper: fill a vector with reverse-ordered bytes (255‑0)    */
void loadRandom(std::vector<int>& pool) {
    int idx = 0;
    while (idx < (int)pool.size()) {
        pool[idx++] = 255 - (idx - 1) % 256;
    }
}

/*------------------------------------------------------------*/
/* Helper: compare two vectors for equality                    */
int checkEqual(std::vector<int>& a, std::vector<int>& b) {
    int pos = 0;
    while (pos < (int)a.size()) {
        if (a[pos] != b[pos]) return 0;
        ++pos;
    }
    return 1;
}

/*------------------------------------------------------------*/
/* Helper: print first ten elements of a vector               */
void dumpSample(std::vector<int>& src, char* tag) {
    std::cout << tag << ": ";
    int limit = src.size() < 10 ? src.size() : 10;
    int i = 0;
    while (i < limit) {
        std::cout << src[i] << ' ';
        ++i;
    }
    std::cout << '\n';
}

/*------------------------------------------------------------*/
/* Class‑based 3‑Way Cipher                                    */
class ThreeWayCipher {
public:
    int keyA, keyB, keyC;

    ThreeWayCipher() {
        keyA = std::rand() % 256;
        keyB = std::rand() % 256;
        keyC = std::rand() % 256;
    }

    std::vector<int> encrypt(std::vector<int>& plain) {
        std::vector<int> cipher(plain.size());
        int i = 0;
        while (i < (int)plain.size()) {
            /* fused expression: ((x ^ A) + B) ^ C */
            cipher[i] = ((plain[i] ^ keyA) + keyB) ^ keyC;
            ++i;
        }
        return cipher;
    }

    std::vector<int> decrypt(std::vector<int>& cipher) {
        std::vector<int> plain(cipher.size());
        int i = 0;
        while (i < (int)cipher.size()) {
            /* reverse of the fused expression */
            plain[i] = ((cipher[i] ^ keyC) - keyB) ^ keyA;
            ++i;
        }
        return plain;
    }
};

/*------------------------------------------------------------*/
int main() {
    std::srand(42);  // deterministic seed for reproducible keys

    /* generate a large reverse‑ordered array */
    std::vector<int> raw(N);
    loadRandom(raw);

    /* instantiate cipher and process data */
    ThreeWayCipher engine;
    std::vector<int> enc = engine.encrypt(raw);
    std::vector<int> dec = engine.decrypt(enc);

    /* verify correctness */
    int ok = checkEqual(raw, dec);

    /* show samples */
    char lblRaw[] = "Original";
    char lblEnc[] = "Encrypted";
    char lblDec[] = "Decrypted";
    dumpSample(raw, lblRaw);
    dumpSample(enc, lblEnc);
    dumpSample(dec, lblDec);

    std::cout << "Verification: " << (ok ? "Success" : "Failure") << '\n';
    return 0;
}
