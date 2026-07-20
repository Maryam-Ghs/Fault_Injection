/* LLM input variant 4: signed-extremes */
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
            unsigned u0 = static_cast<unsigned>(key[0]);
            unsigned u1 = static_cast<unsigned>(key[1]);
            unsigned part = ((u0 << rot) | (u0 >> (32 - rot)))
                           ^ ((u1 >> rot) | (u1 << (32 - rot)));
            subkeys[i] = static_cast<int>(part);
        }
    }

    // Feistel function (very small – just a mix of shifts and adds)
    inline int ffunc(int r, int sk) {
        // use unsigned arithmetic to avoid undefined signed overflow
        unsigned ur = static_cast<unsigned>(r);
        unsigned usk = static_cast<unsigned>(sk);
        unsigned mixed = (ur << 4) + (ur >> 5);
        return static_cast<int>(mixed ^ usk);
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
    // ---------- generate edge‑case keys (heap) ----------
    int *kA = new int[2];                // mixed zero/positive
    int *kB = new int[2];                // all‑ones (‑1)
    int *kC = new int[2];                // mixed positive/negative pattern

    kA[0] = 0;            kA[1] = 123456;          // zero and small positive
    kB[0] = -1;           kB[1] = -1;             // all bits set
    kC[0] = 0x12345678;  kC[1] = -0x12345678;    // positive and negative pattern

    TripleDES tdes(kA, kB, kC);

    // ---------- edge‑case plaintext blocks ----------
    int *p0 = new int[2];   // all zero
    int *p1 = new int[2];   // all ones (‑1)
    int *p2 = new int[2];   // max positive and min negative
    int *p3 = new int[2];   // mixed arbitrary values

    p0[0] = 0;                     p0[1] = 0;
    p1[0] = -1;                    p1[1] = -1;
    p2[0] = 0x7FFFFFFF;            p2[1] = static_cast<int>(0x80000000U); // INT_MAX, INT_MIN
    p3[0] = 123456789;             p3[1] = -987654321;

    int *samples[4] = {p0, p1, p2, p3};

    // ---------- encrypt, display, decrypt, verify ----------
    for (int i = 0; i < 4; ++i) {
        int *blk = samples[i];
        int savedL = blk[0], savedR = blk[1];

        tdes.encryptBlock(blk);
        printf("Sample %d ciphertext: %08X %08X\n", i, static_cast<unsigned>(blk[0]), static_cast<unsigned>(blk[1]));

        tdes.decryptBlock(blk);
        printf("Sample %d recovered : %08X %08X  match:%d\n",
               i, static_cast<unsigned>(blk[0]), static_cast<unsigned>(blk[1]),
               (blk[0] == savedL && blk[1] == savedR));
    }

    // ---------- clean up ----------
    delete[] kA; delete[] kB; delete[] kC;
    delete[] p0; delete[] p1; delete[] p2; delete[] p3;

    return 0;
}
