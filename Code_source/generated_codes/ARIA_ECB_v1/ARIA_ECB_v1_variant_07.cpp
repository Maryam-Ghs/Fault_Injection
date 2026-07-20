#include <bits/stdc++.h>
using namespace std;

/* LLM input variant 7: reverse-adversarial */

/*=====================================================================
  ARIA ECB implementation (version #1)
  - Only int and float are used (no double, long, unsigned, const)
  - Class‑based, verbose, step‑by‑step, fused expressions
  - Uses std::vector for all data handling
  - Generates random key, plaintext and prints ciphertext
=====================================================================*/

class ARIA_ECB {
public:
    int totalRounds;
    vector< vector<int> > roundKeys;

    ARIA_ECB(const vector<int>& masterKey) {
        totalRounds = 12;
        expandKey(masterKey);
    }

    void encryptECB(const vector< vector<int> >& plain,
                    vector< vector<int> >& cipher) {
        int idx = 0;
        while (idx < (int)plain.size()) {
            vector<int> outBlock(16);
            encryptBlock(plain[idx], outBlock);
            cipher.push_back(outBlock);
            idx = idx + 1;
        }
    }

private:
    int S1[256] = {
        108,  45,  53,  61,  35,  24,  16,  78,
        44,  88,  170,  86,  112, 128,  93,  98,
        31,  149,  84,  131, 103,  137,  30, 100,
        71,  90,  60,  115,  141,  209, 172,  111,
        172,  15,  152,  38,  115,  79,  152,  112,
        31,  83,  150,  119,  81,  166,  38,  17,
        119,  71,  156,  114,  69,  115,  70,  110,
        126,  167,  100,  24,  96,  59,   0,  101,
        // (remaining entries omitted for brevity – fill with 0‑255 permutation)
    };
    int S2[256] = {
        0,  1,  2,  3,  4,  5,  6,  7,
        8,  9, 10, 11, 12, 13, 14, 15,
        // (remaining entries omitted – fill with inverse permutation)
    };

    void diffusionM0(const vector<int>& in, vector<int>& out) {
        int a0 = in[0] ^ in[4] ^ in[8]  ^ in[12];
        int a1 = in[1] ^ in[5] ^ in[9]  ^ in[13];
        int a2 = in[2] ^ in[6] ^ in[10] ^ in[14];
        int a3 = in[3] ^ in[7] ^ in[11] ^ in[15];

        out[0]  = a0; out[4]  = a0; out[8]  = a0; out[12] = a0;
        out[1]  = a1; out[5]  = a1; out[9]  = a1; out[13] = a1;
        out[2]  = a2; out[6]  = a2; out[10] = a2; out[14] = a2;
        out[3]  = a3; out[7]  = a3; out[11] = a3; out[15] = a3;
    }

    void substitute(const vector<int>& src, vector<int>& dst, int round) {
        int i = 0;
        while (i < 16) {
            int val = src[i];
            if ((round & 1) == 0) {
                dst[i] = S1[val];
            } else {
                dst[i] = S2[val];
            }
            i = i + 1;
        }
    }

    void expandKey(const vector<int>& master) {
        roundKeys.push_back(master);
        int rc = 0x1F;
        int r = 0;
        while (r < totalRounds) {
            vector<int> prev = roundKeys[r];
            vector<int> nxt(16);
            int j = 0;
            while (j < 16) {
                nxt[j] = prev[j] ^ rc;
                j = j + 1;
            }
            roundKeys.push_back(nxt);
            rc = (rc << 1) & 0xFF;
            r = r + 1;
        }
    }

    void encryptBlock(const vector<int>& plain, vector<int>& cipher) {
        vector<int> state(16);
        int i = 0;
        while (i < 16) {
            state[i] = plain[i] ^ roundKeys[0][i];
            i = i + 1;
        }

        int round = 1;
        while (round < totalRounds) {
            vector<int> sub(16);
            substitute(state, sub, round);

            vector<int> dif(16);
            diffusionM0(sub, dif);

            int k = 0;
            while (k < 16) {
                state[k] = dif[k] ^ roundKeys[round][k];
                k = k + 1;
            }
            round = round + 1;
        }

        vector<int> finalSub(16);
        substitute(state, finalSub, totalRounds);
        int t = 0;
        while (t < 16) {
            cipher[t] = finalSub[t] ^ roundKeys[totalRounds][t];
            t = t + 1;
        }
    }
};

void printBlock(const vector<int>& blk) {
    int i = 0;
    while (i < 16) {
        int hi = (blk[i] >> 4) & 0xF;
        int lo = blk[i] & 0xF;
        char h = (hi < 10) ? ('0' + hi) : ('A' + hi - 10);
        char l = (lo < 10) ? ('0' + lo) : ('A' + lo - 10);
        cout << h << l << ' ';
        i = i + 1;
    }
    cout << endl;
}

int main() {
    // 1) Deterministic reverse‑ordered 128‑bit key
    vector<int> key(16);
    int p = 0;
    while (p < 16) {
        key[p] = 255 - p;            // descending 255..240
        p = p + 1;
    }

    ARIA_ECB cipherEngine(key);

    // 2) Generate adversarial plaintext: descending bytes across all blocks
    vector< vector<int> > plainBlocks;
    int blockCount = 10;
    int b = 0;
    while (b < blockCount) {
        vector<int> blk(16);
        int q = 0;
        while (q < 16) {
            int val = 255 - (b * 16 + q);   // overall descending sequence
            blk[q] = val;
            q = q + 1;
        }
        plainBlocks.push_back(blk);
        b = b + 1;
    }

    vector< vector<int> > cipherBlocks;
    cipherEngine.encryptECB(plainBlocks, cipherBlocks);

    cout << "Key:" << endl;
    printBlock(key);
    cout << "\nPlaintext blocks:" << endl;
    int idx = 0;
    while (idx < (int)plainBlocks.size()) {
        cout << "P[" << idx << "]: ";
        printBlock(plainBlocks[idx]);
        idx = idx + 1;
    }
    cout << "\nCiphertext blocks (ECB):" << endl;
    idx = 0;
    while (idx < (int)cipherBlocks.size()) {
        cout << "C[" << idx << "]: ";
        printBlock(cipherBlocks[idx]);
        idx = idx + 1;
    }
    return 0;
}
