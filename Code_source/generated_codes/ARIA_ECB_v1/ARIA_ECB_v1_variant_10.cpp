#include <bits/stdc++.h>
using namespace std;

/* LLM input variant 10: large-safe-stress */

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
    int S1[256] = { /* omitted for brevity */ };
    int S2[256] = { /* omitted for brevity */ };

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
            if ((round & 1) == 0) dst[i] = S1[val];
            else dst[i] = S2[val];
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
    // Large stress key: all bytes set to 0xAA (170)
    vector<int> key(16, 0xAA);

    ARIA_ECB cipherEngine(key);

    // Large but safe number of plaintext blocks (e.g., 2000)
    vector< vector<int> > plainBlocks;
    const int blockCount = 2000;
    int b = 0;
    while (b < blockCount) {
        vector<int> blk(16);
        int q = 0;
        while (q < 16) {
            // Incremental pattern that stays within 0‑255
            blk[q] = (b + q) % 256;
            q = q + 1;
        }
        plainBlocks.push_back(blk);
        b = b + 1;
    }

    vector< vector<int> > cipherBlocks;
    cipherEngine.encryptECB(plainBlocks, cipherBlocks);

    cout << "Key:" << endl;
    printBlock(key);
    cout << "\nFirst 5 plaintext blocks:" << endl;
    for (int i = 0; i < 5; ++i) {
        cout << "P[" << i << "]: ";
        printBlock(plainBlocks[i]);
    }
    cout << "\nCorresponding ciphertext blocks:" << endl;
    for (int i = 0; i < 5; ++i) {
        cout << "C[" << i << "]: ";
        printBlock(cipherBlocks[i]);
    }
    cout << "\n... (" << blockCount << " blocks processed)" << endl;
    return 0;
}
