/* LLM input variant 2: small-diverse */
// Triple‑DES (EDE) – version #9
// ------------------------------------------------------------
// Implements 3DES using only int, heap allocation, manual loop
// unrolling and reordered arithmetic.  Edge‑case inputs are
// generated inside main() and the results are printed.
//
// ------------------------------------------------------------
#include <cstdio>

class DES {
public:
    int *subkeys;                     // 16 sub‑keys (heap allocated)

    // constructor – builds sub‑keys from a 64‑bit key (2 ints)
    DES(int *key) {
        subkeys = new int[16];
        genSubkeys(key);
    }

    // destructor – free heap memory
    ~DES() {
        delete[] subkeys;
    }

    // simple key schedule – rotate and mix the two halves
    void genSubkeys(int *key) {
        for (int i = 0; i < 16; ++i) {
            int rot = (i * 3) & 31;                     // rotate amount (0‑31)
            int part = ((key[0] << rot) | (key[0] >> (32 - rot)))
                     ^ ((key[1] >> rot) | (key[1] << (32 - rot)));
            subkeys[i] = part;
        }
    }

    // Feistel function (very small – just a mix of shifts and adds)
    inline int ffunc(int r, int sk) {
        // reordered arithmetic: (r << 4) + (r >> 5)  instead of (r >> 5) + (r << 4)
        return ((r << 4) + (r >> 5)) ^ sk;
    }

    // Encrypt a 64‑bit block (L,R) – 16 rounds, manually unrolled
    void encrypt(int &L, int &R) {
        int tmp, sw;

        // round 0
        tmp = ffunc(R, subkeys[0]); L ^= tmp; sw = L; L = R; R = sw;
        // round 1
        tmp = ffunc(R, subkeys[1]); L ^= tmp; sw = L; L = R; R = sw;
        // round 2
        tmp = ffunc(R, subkeys[2]); L ^= tmp; sw = L; L = R; R = sw;
        // round 3
        tmp = ffunc(R, subkeys[3]); L ^= tmp; sw = L; L = R; R = sw;
        // round 4
        tmp = ffunc(R, subkeys[4]); L ^= tmp; sw = L; L = R; R = sw;
        // round 5
        tmp = ffunc(R, subkeys[5]); L ^= tmp; sw = L; L = R; R = sw;
        // round 6
        tmp = ffunc(R, subkeys[6]); L ^= tmp; sw = L; L = R; R = sw;
        // round 7
        tmp = ffunc(R, subkeys[7]); L ^= tmp; sw = L; L = R; R = sw;
        // round 8
        tmp = ffunc(R, subkeys[8]); L ^= tmp; sw = L; L = R; R = sw;
        // round 9
        tmp = ffunc(R, subkeys[9]); L ^= tmp; sw = L; L = R; R = sw;
        // round10
        tmp = ffunc(R, subkeys[10]); L ^= tmp; sw = L; L = R; R = sw;
        // round11
        tmp = ffunc(R, subkeys[11]); L ^= tmp; sw = L; L = R; R = sw;
        // round12
        tmp = ffunc(R, subkeys[12]); L ^= tmp; sw = L; L = R; R = sw;
        // round13
        tmp = ffunc(R, subkeys[13]); L ^= tmp; sw = L; L = R; R = sw;
        // round14
        tmp = ffunc(R, subkeys[14]); L ^= tmp; sw = L; L = R; R = sw;
        // round15
        tmp = ffunc(R, subkeys[15]); L ^= tmp;
        // undo final swap
        sw = L; L = R; R = sw;
    }

    // Decrypt a 64‑bit block – reverse round order
    void decrypt(int &L, int &R) {
        int tmp, sw;

        // round15
        tmp = ffunc(R, subkeys[15]); L ^= tmp; sw = L; L = R; R = sw;
        // round14
        tmp = ffunc(R, subkeys[14]); L ^= tmp; sw = L; L = R; R = sw;
        // round13
        tmp = ffunc(R, subkeys[13]); L ^= tmp; sw = L; L = R; R = sw;
        // round12
        tmp = ffunc(R, subkeys[12]); L ^= tmp; sw = L; L = R; R = sw;
        // round11
        tmp = ffunc(R, subkeys[11]); L ^= tmp; sw = L; L = R; R = sw;
        // round10
        tmp = ffunc(R, subkeys[10]); L ^= tmp; sw = L; L = R; R = sw;
        // round9
        tmp = ffunc(R, subkeys[9]); L ^= tmp; sw = L; L = R; R = sw;
        // round8
        tmp = ffunc(R, subkeys[8]); L ^= tmp; sw = L; L = R; R = sw;
        // round7
        tmp = ffunc(R, subkeys[7]); L ^= tmp; sw = L; L = R; R = sw;
        // round6
        tmp = ffunc(R, subkeys[6]); L ^= tmp; sw = L; L = R; R = sw;
        // round5
        tmp = ffunc(R, subkeys[5]); L ^= tmp; sw = L; L = R; R = sw;
        // round4
        tmp = ffunc(R, subkeys[4]); L ^= tmp; sw = L; L = R; R = sw;
        // round3
        tmp = ffunc(R, subkeys[3]); L ^= tmp; sw = L; L = R; R = sw;
        // round2
        tmp = ffunc(R, subkeys[2]); L ^= tmp; sw = L; L = R; R = sw;
        // round1
        tmp = ffunc(R, subkeys[1]); L ^= tmp; sw = L; L = R; R = sw;
        // round0
        tmp = ffunc(R, subkeys[0]); L ^= tmp;
        // undo final swap
        sw = L; L = R; R = sw;
    }
};

class TripleDES {
public:
    DES *d1, *d2, *d3;

    TripleDES(int *k1, int *k2, int *k3) {
        d1 = new DES(k1);
        d2 = new DES(k2);
        d3 = new DES(k3);
    }

    ~TripleDES() {
        delete d1;
        delete d2;
        delete d3;
    }

    // EDE sequence: encrypt – decrypt – encrypt
    void encryptBlock(int *blk) {
        int L = blk[0], R = blk[1];
        d1->encrypt(L, R);
        d2->decrypt(L, R);
        d3->encrypt(L, R);
        blk[0] = L; blk[1] = R;
    }

    // DED sequence: decrypt – encrypt – decrypt
    void decryptBlock(int *blk) {
        int L = blk[0], R = blk[1];
        d3->decrypt(L, R);
        d2->encrypt(L, R);
        d1->decrypt(L, R);
        blk[0] = L; blk[1] = R;
    }
};

int main() {
    // ---------- generate small‑diverse keys (heap) ----------
    int *kA = new int[2];                // mixed pattern key
    int *kB = new int[2];                // complement pattern key
    int *kC = new int[2];                // high‑low pattern key

    kA[0] = 0x12345678;  kA[1] = 0x9ABCDEF0;
    kB[0] = 0x0F0F0F0F;  kB[1] = 0xF0F0F0F0;
    kC[0] = 0xFFFFFFFF;  kC[1] = 0x00000000;

    TripleDES tdes(kA, kB, kC);

    // ---------- small‑diverse plaintext blocks ----------
    int *p0 = new int[2];   // pattern 1
    int *p1 = new int[2];   // pattern 2
    int *p2 = new int[2];   // pattern 3
    int *p3 = new int[2];   // pattern 4

    p0[0] = 0x11111111;  p0[1] = 0x22222222;
    p1[0] = 0x33333333;  p1[1] = 0x44444444;
    p2[0] = 0x55555555;  p2[1] = 0xAAAAAAAA;
    p3[0] = 0x80000000;  p3[1] = 0x7FFFFFFF;

    int *samples[4] = {p0, p1, p2, p3};

    // ---------- encrypt, display, decrypt, verify ----------
    for (int i = 0; i < 4; ++i) {
        int *blk = samples[i];
        int savedL = blk[0], savedR = blk[1];

        tdes.encryptBlock(blk);
        printf("Sample %d ciphertext: %08X %08X\n", i, blk[0], blk[1]);

        tdes.decryptBlock(blk);
        printf("Sample %d recovered : %08X %08X  match:%d\n",
               i, blk[0], blk[1],
               (blk[0] == savedL && blk[1] == savedR));
    }

    // ---------- clean up ----------
    delete[] kA; delete[] kB; delete[] kC;
    delete[] p0; delete[] p1; delete[] p2; delete[] p3;

    return 0;
}
