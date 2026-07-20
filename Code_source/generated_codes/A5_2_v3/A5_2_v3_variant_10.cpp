#include <iostream>
#include <cstdlib>
#include <ctime>

/* LLM input variant 10: large-safe-stress */

int main() {
    // --------------------------------------------------------------
    // 1. Generate pseudo‑random key (64 bits) and frame number (22 bits)
    // --------------------------------------------------------------
    std::srand(12345);                     // fixed seed for reproducibility
    int keyBits[64];
    int frmBits[22];
    for (int i = 0; i < 64; ++i) keyBits[i] = std::rand() & 1;
    for (int i = 0; i < 22; ++i) frmBits[i] = std::rand() & 1;

    // --------------------------------------------------------------
    // 2. Stack‑allocated LFSR state (19, 22, 23 bits)
    // --------------------------------------------------------------
    int R1[19] = {0};
    int R2[22] = {0};
    int R3[23] = {0};

    // --------------------------------------------------------------
    // 3. Load key into the three registers (manual unrolling)
    // --------------------------------------------------------------
    // first 19 bits → R1
    R1[0] ^= keyBits[0];   R1[1] ^= keyBits[1];   R1[2] ^= keyBits[2];
    R1[3] ^= keyBits[3];   R1[4] ^= keyBits[4];   R1[5] ^= keyBits[5];
    R1[6] ^= keyBits[6];   R1[7] ^= keyBits[7];   R1[8] ^= keyBits[8];
    R1[9] ^= keyBits[9];   R1[10] ^= keyBits[10]; R1[11] ^= keyBits[11];
    R1[12] ^= keyBits[12]; R1[13] ^= keyBits[13]; R1[14] ^= keyBits[14];
    R1[15] ^= keyBits[15]; R1[16] ^= keyBits[16]; R1[17] ^= keyBits[17];
    R1[18] ^= keyBits[18];

    // next 22 bits → R2
    R2[0] ^= keyBits[19];  R2[1] ^= keyBits[20];  R2[2] ^= keyBits[21];
    R2[3] ^= keyBits[22];  R2[4] ^= keyBits[23];  R2[5] ^= keyBits[24];
    R2[6] ^= keyBits[25];  R2[7] ^= keyBits[26];  R2[8] ^= keyBits[27];
    R2[9] ^= keyBits[28];  R2[10] ^= keyBits[29]; R2[11] ^= keyBits[30];
    R2[12] ^= keyBits[31]; R2[13] ^= keyBits[32]; R2[14] ^= keyBits[33];
    R2[15] ^= keyBits[34]; R2[16] ^= keyBits[35]; R2[17] ^= keyBits[36];
    R2[18] ^= keyBits[37]; R2[19] ^= keyBits[38]; R2[20] ^= keyBits[39];
    R2[21] ^= keyBits[40];

    // remaining 13 bits → R3
    R3[0] ^= keyBits[41];  R3[1] ^= keyBits[42];  R3[2] ^= keyBits[43];
    R3[3] ^= keyBits[44];  R3[4] ^= keyBits[45];  R3[5] ^= keyBits[46];
    R3[6] ^= keyBits[47];  R3[7] ^= keyBits[48];  R3[8] ^= keyBits[49];
    R3[9] ^= keyBits[50];  R3[10] ^= keyBits[51]; R3[11] ^= keyBits[52];
    R3[12] ^= keyBits[53]; R3[13] ^= keyBits[54]; R3[14] ^= keyBits[55];
    R3[15] ^= keyBits[56]; R3[16] ^= keyBits[57]; R3[17] ^= keyBits[58];
    R3[18] ^= keyBits[59]; R3[19] ^= keyBits[60]; R3[20] ^= keyBits[61];
    R3[21] ^= keyBits[62]; R3[22] ^= keyBits[63];

    // --------------------------------------------------------------
    // 4. Load frame number (same unrolled pattern)
    // --------------------------------------------------------------
    // first 19 bits of frame → R1 (only 19 bits exist, so use all)
    R1[0] ^= frmBits[0];   R1[1] ^= frmBits[1];   R1[2] ^= frmBits[2];
    R1[3] ^= frmBits[3];   R1[4] ^= frmBits[4];   R1[5] ^= frmBits[5];
    R1[6] ^= frmBits[6];   R1[7] ^= frmBits[7];   R1[8] ^= frmBits[8];
    R1[9] ^= frmBits[9];   R1[10] ^= frmBits[10]; R1[11] ^= frmBits[11];
    R1[12] ^= frmBits[12]; R1[13] ^= frmBits[13]; R1[14] ^= frmBits[14];
    R1[15] ^= frmBits[15]; R1[16] ^= frmBits[16]; R1[17] ^= frmBits[17];
    R1[18] ^= frmBits[18];

    // remaining 3 bits → R2
    R2[0] ^= frmBits[19];  R2[1] ^= frmBits[20];  R2[2] ^= frmBits[21];

    // --------------------------------------------------------------
    // 5. Helper lambdas for LFSR feedback (polynomials of A5/2)
    // --------------------------------------------------------------
    auto fb1 = [&]() -> int { return R1[13] ^ R1[16] ^ R1[17] ^ R1[18]; };
    auto fb2 = [&]() -> int { return R2[20] ^ R2[21]; };
    auto fb3 = [&]() -> int { return R3[7] ^ R3[20] ^ R3[21] ^ R3[22]; };

    // --------------------------------------------------------------
    // 6. Mixing phase – 10000 cycles, irregular clocking via majority
    //    (manual loop unrolling, fused operations)
    // --------------------------------------------------------------
    int mixCnt = 10000;
    while (mixCnt--) {
        // majority of the three clock‑control bits
        int maj = (R1[8] + R2[10] + R3[10] > 1) ? 1 : 0;

        // clock R1 if its control bit equals majority
        if (R1[8] == maj) {
            int nxt = fb1();
            // shift right, insert new bit at position 0
            R1[18] = R1[17]; R1[17] = R1[16]; R1[16] = R1[15];
            R1[15] = R1[14]; R1[14] = R1[13]; R1[13] = R1[12];
            R1[12] = R1[11]; R1[11] = R1[10]; R1[10] = R1[9];
            R1[9]  = R1[8];  R1[8]  = R1[7];  R1[7]  = R1[6];
            R1[6]  = R1[5];  R1[5]  = R1[4];  R1[4]  = R1[3];
            R1[3]  = R1[2];  R1[2]  = R1[1];  R1[1]  = R1[0];
            R1[0] = nxt;
        }

        // clock R2 if its control bit equals majority
        if (R2[10] == maj) {
            int nxt = fb2();
            R2[21] = R2[20]; R2[20] = R2[19]; R2[19] = R2[18];
            R2[18] = R2[17]; R2[17] = R2[16]; R2[16] = R2[15];
            R2[15] = R2[14]; R2[14] = R2[13]; R2[13] = R2[12];
            R2[12] = R2[11]; R2[11] = R2[10]; R2[10] = R2[9];
            R2[9]  = R2[8];  R2[8]  = R2[7];  R2[7]  = R2[6];
            R2[6]  = R2[5];  R2[5]  = R2[4];  R2[4]  = R2[3];
            R2[3]  = R2[2];  R2[2]  = R2[1];  R2[1]  = R2[0];
            R2[0] = nxt;
        }

        // clock R3 if its control bit equals majority
        if (R3[10] == maj) {
            int nxt = fb3();
            R3[22] = R3[21]; R3[21] = R3[20]; R3[20] = R3[19];
            R3[19] = R3[18]; R3[18] = R3[17]; R3[17] = R3[16];
            R3[16] = R3[15]; R3[15] = R3[14]; R3[14] = R3[13];
            R3[13] = R3[12]; R3[12] = R3[11]; R3[11] = R3[10];
            R3[10] = R3[9];  R3[9]  = R3[8];  R3[8]  = R3[7];
            R3[7]  = R3[6];  R3[6]  = R3[5];  R3[5]  = R3[4];
            R3[4]  = R3[3];  R3[3]  = R3[2];  R3[2]  = R3[1];
            R3[1]  = R3[0];  R3[0]  = nxt;
        }
    }

    // --------------------------------------------------------------
    // 7. Keystream generation – produce 1000 bits (fused output expression)
    // --------------------------------------------------------------
    const int ksLen = 1000;
    int ks[ksLen];
    for (int i = 0; i < ksLen; ++i) {
        // output = (R1[18] ^ R2[21]) ^ R3[22]  (simple fused XOR)
        ks[i] = (R1[18] ^ R2[21]) ^ R3[22];

        // clock all three registers (regular clocking for output phase)
        int n1 = fb1(); int n2 = fb2(); int n3 = fb3();

        // shift R1
        R1[18] = R1[17]; R1[17] = R1[16]; R1[16] = R1[15];
        R1[15] = R1[14]; R1[14] = R1[13]; R1[13] = R1[12];
        R1[12] = R1[11]; R1[11] = R1[10]; R1[10] = R1[9];
        R1[9]  = R1[8];  R1[8]  = R1[7];  R1[7]  = R1[6];
        R1[6]  = R1[5];  R1[5]  = R1[4];  R1[4]  = R1[3];
        R1[3]  = R1[2];  R1[2]  = R1[1];  R1[1]  = R1[0];
        R1[0] = n1;

        // shift R2
        R2[21] = R2[20]; R2[20] = R2[19]; R2[19] = R2[18];
        R2[18] = R2[17]; R2[17] = R2[16]; R2[16] = R2[15];
        R2[15] = R2[14]; R2[14] = R2[13]; R2[13] = R2[12];
        R2[12] = R2[11]; R2[11] = R2[10]; R2[10] = R2[9];
        R2[9]  = R2[8];  R2[8]  = R2[7];  R2[7]  = R2[6];
        R2[6]  = R2[5];  R2[5]  = R2[4];  R2[4]  = R2[3];
        R2[3]  = R2[2];  R2[2]  = R2[1];  R2[1]  = R2[0];
        R2[0] = n2;

        // shift R3
        R3[22] = R3[21]; R3[21] = R3[20]; R3[20] = R3[19];
        R3[19] = R3[18]; R3[18] = R3[17]; R3[17] = R3[16];
        R3[16] = R3[15]; R3[15] = R3[14]; R3[14] = R3[13];
        R3[13] = R3[12]; R3[12] = R3[11]; R3[11] = R3[10];
        R3[10] = R3[9];  R3[9]  = R3[8];  R3[8]  = R3[7];
        R3[7]  = R3[6];  R3[6]  = R3[5];  R3[5]  = R3[4];
        R3[4]  = R3[3];  R3[3]  = R3[2];  R3[2]  = R3[1];
        R3[1]  = R3[0];  R3[0] = n3;
    }

    // --------------------------------------------------------------
    // 8. Print key, frame and resulting keystream
    // --------------------------------------------------------------
    std::cout << "Key (64 bits)   : ";
    for (int i = 0; i < 64; ++i) std::cout << keyBits[i];
    std::cout << "\nFrame (22 bits) : ";
    for (int i = 0; i < 22; ++i) std::cout << frmBits[i];
    std::cout << "\nKeystream (" << ksLen << ") : ";
    for (int i = 0; i < ksLen; ++i) std::cout << ks[i];
    std::cout << std::endl;

    return 0;
}
