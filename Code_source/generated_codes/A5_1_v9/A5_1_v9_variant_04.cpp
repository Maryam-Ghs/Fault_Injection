/* LLM input variant 4: signed-extremes */
/* A5/1 stream cipher – version #9 – all logic inside main()
   Constraints: only int/float, no const, no unsigned, no cin.
   Deterministic test vectors, manual loop unrolling, reordered arithmetic,
   stack‑allocated arrays. */

#include <iostream>

int main() {
    // ------------------------------------------------------------
    // 1.  Registers (19, 22, 23 bits) – deterministic start values
    // ------------------------------------------------------------
    int regA = 0x0;        // zero start for 19‑bit LFSR
    int regB = 0x2AAAAA;   // 22‑bit LFSR, pattern 0010101010101010101010
    int regC = 0x3FFFFF;   // max 22‑bit value for 23‑bit LFSR (still within signed int)

    // ------------------------------------------------------------
    // 2.  Masks for each register (keep them inside 32‑bit signed int)
    // ------------------------------------------------------------
    #define M19 0x7FFFF   // (1<<19)-1
    #define M22 0x3FFFFF  // (1<<22)-1
    #define M23 0x7FFFFF  // (1<<23)-1

    // ------------------------------------------------------------
    // 3.  Output buffer – we will generate 32 keystream bits
    // ------------------------------------------------------------
    int ksBits[32];
    
    // ------------------------------------------------------------
    // 4.  Helper lambdas (still inside main) – they use only int arithmetic
    // ------------------------------------------------------------
    auto outBitA = [] (int r) { return (r >> 18) & 1; };
    auto outBitB = [] (int r) { return (r >> 21) & 1; };
    auto outBitC = [] (int r) { return (r >> 22) & 1; };
    auto clkBitA = [] (int r) { return (r >> 8)  & 1; };
    auto clkBitB = [] (int r) { return (r >> 10) & 1; };
    auto clkBitC = [] (int r) { return (r >> 10) & 1; };

    // ------------------------------------------------------------
    // 5.  Dummy signed‑extremes data (does not affect keystream)
    // ------------------------------------------------------------
    int signedVals[5] = { -12345, 0, 6789, -42, 0 };
    int dummySum = 0;
    for (int i = 0; i < 5; ++i) {
        dummySum += signedVals[i];
    }
    // (dummySum is intentionally unused in further computation)

    // ------------------------------------------------------------
    // 6.  Manual unrolled generation of 32 keystream bits
    //    (the same code block repeated 32 times)
    // ------------------------------------------------------------
    // ---- step 0 -------------------------------------------------
    {
        int vA = clkBitA(regA);
        int vB = clkBitB(regB);
        int vC = clkBitC(regC);
        int sum = vA + vB + vC;               // reordered arithmetic
        int maj = (sum >= 2);                 // majority function

        // shift register A if its clock bit matches majority
        if (vA == maj) {
            int fb = ((regA >> 13) ^ (regA >> 16) ^ (regA >> 17) ^ (regA >> 18)) & 1;
            regA = ((regA << 1) & M19) | fb;   // left shift, mask, insert feedback
        }
        // shift register B if its clock bit matches majority
        if (vB == maj) {
            int fb = ((regB >> 20) ^ (regB >> 21)) & 1;
            regB = ((regB << 1) & M22) | fb;
        }
        // shift register C if its clock bit matches majority
        if (vC == maj) {
            int fb = ((regC >> 7) ^ (regC >> 20) ^ (regC >> 21) ^ (regC >> 22)) & 1;
            regC = ((regC << 1) & M23) | fb;
        }
        ksBits[0] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 1 -------------------------------------------------
    {
        int vA = clkBitA(regA);
        int vB = clkBitB(regB);
        int vC = clkBitC(regC);
        int sum = vA + vB + vC;
        int maj = (sum >= 2);

        if (vA == maj) {
            int fb = ((regA >> 13) ^ (regA >> 16) ^ (regA >> 17) ^ (regA >> 18)) & 1;
            regA = ((regA << 1) & M19) | fb;
        }
        if (vB == maj) {
            int fb = ((regB >> 20) ^ (regB >> 21)) & 1;
            regB = ((regB << 1) & M22) | fb;
        }
        if (vC == maj) {
            int fb = ((regC >> 7) ^ (regC >> 20) ^ (regC >> 21) ^ (regC >> 22)) & 1;
            regC = ((regC << 1) & M23) | fb;
        }
        ksBits[1] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 2 -------------------------------------------------
    {
        int vA = clkBitA(regA);
        int vB = clkBitB(regB);
        int vC = clkBitC(regC);
        int sum = vA + vB + vC;
        int maj = (sum >= 2);

        if (vA == maj) {
            int fb = ((regA >> 13) ^ (regA >> 16) ^ (regA >> 17) ^ (regA >> 18)) & 1;
            regA = ((regA << 1) & M19) | fb;
        }
        if (vB == maj) {
            int fb = ((regB >> 20) ^ (regB >> 21)) & 1;
            regB = ((regB << 1) & M22) | fb;
        }
        if (vC == maj) {
            int fb = ((regC >> 7) ^ (regC >> 20) ^ (regC >> 21) ^ (regC >> 22)) & 1;
            regC = ((regC << 1) & M23) | fb;
        }
        ksBits[2] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 3 -------------------------------------------------
    {
        int vA = clkBitA(regA);
        int vB = clkBitB(regB);
        int vC = clkBitC(regC);
        int sum = vA + vB + vC;
        int maj = (sum >= 2);

        if (vA == maj) {
            int fb = ((regA >> 13) ^ (regA >> 16) ^ (regA >> 17) ^ (regA >> 18)) & 1;
            regA = ((regA << 1) & M19) | fb;
        }
        if (vB == maj) {
            int fb = ((regB >> 20) ^ (regB >> 21)) & 1;
            regB = ((regB << 1) & M22) | fb;
        }
        if (vC == maj) {
            int fb = ((regC >> 7) ^ (regC >> 20) ^ (regC >> 21) ^ (regC >> 22)) & 1;
            regC = ((regC << 1) & M23) | fb;
        }
        ksBits[3] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 4 -------------------------------------------------
    {
        int vA = clkBitA(regA);
        int vB = clkBitB(regB);
        int vC = clkBitC(regC);
        int sum = vA + vB + vC;
        int maj = (sum >= 2);

        if (vA == maj) {
            int fb = ((regA >> 13) ^ (regA >> 16) ^ (regA >> 17) ^ (regA >> 18)) & 1;
            regA = ((regA << 1) & M19) | fb;
        }
        if (vB == maj) {
            int fb = ((regB >> 20) ^ (regB >> 21)) & 1;
            regB = ((regB << 1) & M22) | fb;
        }
        if (vC == maj) {
            int fb = ((regC >> 7) ^ (regC >> 20) ^ (regC >> 21) ^ (regC >> 22)) & 1;
            regC = ((regC << 1) & M23) | fb;
        }
        ksBits[4] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 5 -------------------------------------------------
    {
        int vA = clkBitA(regA);
        int vB = clkBitB(regB);
        int vC = clkBitC(regC);
        int sum = vA + vB + vC;
        int maj = (sum >= 2);

        if (vA == maj) {
            int fb = ((regA >> 13) ^ (regA >> 16) ^ (regA >> 17) ^ (regA >> 18)) & 1;
            regA = ((regA << 1) & M19) | fb;
        }
        if (vB == maj) {
            int fb = ((regB >> 20) ^ (regB >> 21)) & 1;
            regB = ((regB << 1) & M22) | fb;
        }
        if (vC == maj) {
            int fb = ((regC >> 7) ^ (regC >> 20) ^ (regC >> 21) ^ (regC >> 22)) & 1;
            regC = ((regC << 1) & M23) | fb;
        }
        ksBits[5] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 6 -------------------------------------------------
    {
        int vA = clkBitA(regA);
        int vB = clkBitB(regB);
        int vC = clkBitC(regC);
        int sum = vA + vB + vC;
        int maj = (sum >= 2);

        if (vA == maj) {
            int fb = ((regA >> 13) ^ (regA >> 16) ^ (regA >> 17) ^ (regA >> 18)) & 1;
            regA = ((regA << 1) & M19) | fb;
        }
        if (vB == maj) {
            int fb = ((regB >> 20) ^ (regB >> 21)) & 1;
            regB = ((regB << 1) & M22) | fb;
        }
        if (vC == maj) {
            int fb = ((regC >> 7) ^ (regC >> 20) ^ (regC >> 21) ^ (regC >> 22)) & 1;
            regC = ((regC << 1) & M23) | fb;
        }
        ksBits[6] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 7 -------------------------------------------------
    {
        int vA = clkBitA(regA);
        int vB = clkBitB(regB);
        int vC = clkBitC(regC);
        int sum = vA + vB + vC;
        int maj = (sum >= 2);

        if (vA == maj) {
            int fb = ((regA >> 13) ^ (regA >> 16) ^ (regA >> 17) ^ (regA >> 18)) & 1;
            regA = ((regA << 1) & M19) | fb;
        }
        if (vB == maj) {
            int fb = ((regB >> 20) ^ (regB >> 21)) & 1;
            regB = ((regB << 1) & M22) | fb;
        }
        if (vC == maj) {
            int fb = ((regC >> 7) ^ (regC >> 20) ^ (regC >> 21) ^ (regC >> 22)) & 1;
            regC = ((regC << 1) & M23) | fb;
        }
        ksBits[7] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 8 -------------------------------------------------
    {
        int vA = clkBitA(regA);
        int vB = clkBitB(regB);
        int vC = clkBitC(regC);
        int sum = vA + vB + vC;
        int maj = (sum >= 2);

        if (vA == maj) {
            int fb = ((regA >> 13) ^ (regA >> 16) ^ (regA >> 17) ^ (regA >> 18)) & 1;
            regA = ((regA << 1) & M19) | fb;
        }
        if (vB == maj) {
            int fb = ((regB >> 20) ^ (regB >> 21)) & 1;
            regB = ((regB << 1) & M22) | fb;
        }
        if (vC == maj) {
            int fb = ((regC >> 7) ^ (regC >> 20) ^ (regC >> 21) ^ (regC >> 22)) & 1;
            regC = ((regC << 1) & M23) | fb;
        }
        ksBits[8] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 9 -------------------------------------------------
    {
        int vA = clkBitA(regA);
        int vB = clkBitB(regB);
        int vC = clkBitC(regC);
        int sum = vA + vB + vC;
        int maj = (sum >= 2);

        if (vA == maj) {
            int fb = ((regA >> 13) ^ (regA >> 16) ^ (regA >> 17) ^ (regA >> 18)) & 1;
            regA = ((regA << 1) & M19) | fb;
        }
        if (vB == maj) {
            int fb = ((regB >> 20) ^ (regB >> 21)) & 1;
            regB = ((regB << 1) & M22) | fb;
        }
        if (vC == maj) {
            int fb = ((regC >> 7) ^ (regC >> 20) ^ (regC >> 21) ^ (regC >> 22)) & 1;
            regC = ((regC << 1) & M23) | fb;
        }
        ksBits[9] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 10 ------------------------------------------------
    {
        int vA = clkBitA(regA);
        int vB = clkBitB(regB);
        int vC = clkBitC(regC);
        int sum = vA + vB + vC;
        int maj = (sum >= 2);

        if (vA == maj) {
            int fb = ((regA >> 13) ^ (regA >> 16) ^ (regA >> 17) ^ (regA >> 18)) & 1;
            regA = ((regA << 1) & M19) | fb;
        }
        if (vB == maj) {
            int fb = ((regB >> 20) ^ (regB >> 21)) & 1;
            regB = ((regB << 1) & M22) | fb;
        }
        if (vC == maj) {
            int fb = ((regC >> 7) ^ (regC >> 20) ^ (regC >> 21) ^ (regC >> 22)) & 1;
            regC = ((regC << 1) & M23) | fb;
        }
        ksBits[10] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 11 ------------------------------------------------
    {
        int vA = clkBitA(regA);
        int vB = clkBitB(regB);
        int vC = clkBitC(regC);
        int sum = vA + vB + vC;
        int maj = (sum >= 2);

        if (vA == maj) {
            int fb = ((regA >> 13) ^ (regA >> 16) ^ (regA >> 17) ^ (regA >> 18)) & 1;
            regA = ((regA << 1) & M19) | fb;
        }
        if (vB == maj) {
            int fb = ((regB >> 20) ^ (regB >> 21)) & 1;
            regB = ((regB << 1) & M22) | fb;
        }
        if (vC == maj) {
            int fb = ((regC >> 7) ^ (regC >> 20) ^ (regC >> 21) ^ (regC >> 22)) & 1;
            regC = ((regC << 1) & M23) | fb;
        }
        ksBits[11] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 12 ------------------------------------------------
    {
        int vA = clkBitA(regA);
        int vB = clkBitB(regB);
        int vC = clkBitC(regC);
        int sum = vA + vB + vC;
        int maj = (sum >= 2);

        if (vA == maj) {
            int fb = ((regA >> 13) ^ (regA >> 16) ^ (regA >> 17) ^ (regA >> 18)) & 1;
            regA = ((regA << 1) & M19) | fb;
        }
        if (vB == maj) {
            int fb = ((regB >> 20) ^ (regB >> 21)) & 1;
            regB = ((regB << 1) & M22) | fb;
        }
        if (vC == maj) {
            int fb = ((regC >> 7) ^ (regC >> 20) ^ (regC >> 21) ^ (regC >> 22)) & 1;
            regC = ((regC << 1) & M23) | fb;
        }
        ksBits[12] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 13 ------------------------------------------------
    {
        int vA = clkBitA(regA);
        int vB = clkBitB(regB);
        int vC = clkBitC(regC);
        int sum = vA + vB + vC;
        int maj = (sum >= 2);

        if (vA == maj) {
            int fb = ((regA >> 13) ^ (regA >> 16) ^ (regA >> 17) ^ (regA >> 18)) & 1;
            regA = ((regA << 1) & M19) | fb;
        }
        if (vB == maj) {
            int fb = ((regB >> 20) ^ (regB >> 21)) & 1;
            regB = ((regB << 1) & M22) | fb;
        }
        if (vC == maj) {
            int fb = ((regC >> 7) ^ (regC >> 20) ^ (regC >> 21) ^ (regC >> 22)) & 1;
            regC = ((regC << 1) & M23) | fb;
        }
        ksBits[13] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 14 ------------------------------------------------
    {
        int vA = clkBitA(regA);
        int vB = clkBitB(regB);
        int vC = clkBitC(regC);
        int sum = vA + vB + vC;
        int maj = (sum >= 2);

        if (vA == maj) {
            int fb = ((regA >> 13) ^ (regA >> 16) ^ (regA >> 17) ^ (regA >> 18)) & 1;
            regA = ((regA << 1) & M19) | fb;
        }
        if (vB == maj) {
            int fb = ((regB >> 20) ^ (regB >> 21)) & 1;
            regB = ((regB << 1) & M22) | fb;
        }
        if (vC == maj) {
            int fb = ((regC >> 7) ^ (regC >> 20) ^ (regC >> 21) ^ (regC >> 22)) & 1;
            regC = ((regC << 1) & M23) | fb;
        }
        ksBits[14] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 15 ------------------------------------------------
    {
        int vA = clkBitA(regA);
        int vB = clkBitB(regB);
        int vC = clkBitC(regC);
        int sum = vA + vB + vC;
        int maj = (sum >= 2);

        if (vA == maj) {
            int fb = ((regA >> 13) ^ (regA >> 16) ^ (regA >> 17) ^ (regA >> 18)) & 1;
            regA = ((regA << 1) & M19) | fb;
        }
        if (vB == maj) {
            int fb = ((regB >> 20) ^ (regB >> 21)) & 1;
            regB = ((regB << 1) & M22) | fb;
        }
        if (vC == maj) {
            int fb = ((regC >> 7) ^ (regC >> 20) ^ (regC >> 21) ^ (regC >> 22)) & 1;
            regC = ((regC << 1) & M23) | fb;
        }
        ksBits[15] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 16 ------------------------------------------------
    {
        int vA = clkBitA(regA);
        int vB = clkBitB(regB);
        int vC = clkBitC(regC);
        int sum = vA + vB + vC;
        int maj = (sum >= 2);

        if (vA == maj) {
            int fb = ((regA >> 13) ^ (regA >> 16) ^ (regA >> 17) ^ (regA >> 18)) & 1;
            regA = ((regA << 1) & M19) | fb;
        }
        if (vB == maj) {
            int fb = ((regB >> 20) ^ (regB >> 21)) & 1;
            regB = ((regB << 1) & M22) | fb;
        }
        if (vC == maj) {
            int fb = ((regC >> 7) ^ (regC >> 20) ^ (regC >> 21) ^ (regC >> 22)) & 1;
            regC = ((regC << 1) & M23) | fb;
        }
        ksBits[16] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 17 ------------------------------------------------
    {
        int vA = clkBitA(regA);
        int vB = clkBitB(regB);
        int vC = clkBitC(regC);
        int sum = vA + vB + vC;
        int maj = (sum >= 2);

        if (vA == maj) {
            int fb = ((regA >> 13) ^ (regA >> 16) ^ (regA >> 17) ^ (regA >> 18)) & 1;
            regA = ((regA << 1) & M19) | fb;
        }
        if (vB == maj) {
            int fb = ((regB >> 20) ^ (regB >> 21)) & 1;
            regB = ((regB << 1) & M22) | fb;
        }
        if (vC == maj) {
            int fb = ((regC >> 7) ^ (regC >> 20) ^ (regC >> 21) ^ (regC >> 22)) & 1;
            regC = ((regC << 1) & M23) | fb;
        }
        ksBits[17] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 18 ------------------------------------------------
    {
        int vA = clkBitA(regA);
        int vB = clkBitB(regB);
        int vC = clkBitC(regC);
        int sum = vA + vB + vC;
        int maj = (sum >= 2);

        if (vA == maj) {
            int fb = ((regA >> 13) ^ (regA >> 16) ^ (regA >> 17) ^ (regA >> 18)) & 1;
            regA = ((regA << 1) & M19) | fb;
        }
        if (vB == maj) {
            int fb = ((regB >> 20) ^ (regB >> 21)) & 1;
            regB = ((regB << 1) & M22) | fb;
        }
        if (vC == maj) {
            int fb = ((regC >> 7) ^ (regC >> 20) ^ (regC >> 21) ^ (regC >> 22)) & 1;
            regC = ((regC << 1) & M23) | fb;
        }
        ksBits[18] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 19 ------------------------------------------------
    {
        int vA = clkBitA(regA);
        int vB = clkBitB(regB);
        int vC = clkBitC(regC);
        int sum = vA + vB + vC;
        int maj = (sum >= 2);

        if (vA == maj) {
            int fb = ((regA >> 13) ^ (regA >> 16) ^ (regA >> 17) ^ (regA >> 18)) & 1;
            regA = ((regA << 1) & M19) | fb;
        }
        if (vB == maj) {
            int fb = ((regB >> 20) ^ (regB >> 21)) & 1;
            regB = ((regB << 1) & M22) | fb;
        }
        if (vC == maj) {
            int fb = ((regC >> 7) ^ (regC >> 20) ^ (regC >> 21) ^ (regC >> 22)) & 1;
            regC = ((regC << 1) & M23) | fb;
        }
        ksBits[19] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 20 ------------------------------------------------
    {
        int vA = clkBitA(regA);
        int vB = clkBitB(regB);
        int vC = clkBitC(regC);
        int sum = vA + vB + vC;
        int maj = (sum >= 2);

        if (vA == maj) {
            int fb = ((regA >> 13) ^ (regA >> 16) ^ (regA >> 17) ^ (regA >> 18)) & 1;
            regA = ((regA << 1) & M19) | fb;
        }
        if (vB == maj) {
            int fb = ((regB >> 20) ^ (regB >> 21)) & 1;
            regB = ((regB << 1) & M22) | fb;
        }
        if (vC == maj) {
            int fb = ((regC >> 7) ^ (regC >> 20) ^ (regC >> 21) ^ (regC >> 22)) & 1;
            regC = ((regC << 1) & M23) | fb;
        }
        ksBits[20] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 21 ------------------------------------------------
    {
        int vA = clkBitA(regA);
        int vB = clkBitB(regB);
        int vC = clkBitC(regC);
        int sum = vA + vB + vC;
        int maj = (sum >= 2);

        if (vA == maj) {
            int fb = ((regA >> 13) ^ (regA >> 16) ^ (regA >> 17) ^ (regA >> 18)) & 1;
            regA = ((regA << 1) & M19) | fb;
        }
        if (vB == maj) {
            int fb = ((regB >> 20) ^ (regB >> 21)) & 1;
            regB = ((regB << 1) & M22) | fb;
        }
        if (vC == maj) {
            int fb = ((regC >> 7) ^ (regC >> 20) ^ (regC >> 21) ^ (regC >> 22)) & 1;
            regC = ((regC << 1) & M23) | fb;
        }
        ksBits[21] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 22 ------------------------------------------------
    {
        int vA = clkBitA(regA);
        int vB = clkBitB(regB);
        int vC = clkBitC(regC);
        int sum = vA + vB + vC;
        int maj = (sum >= 2);

        if (vA == maj) {
            int fb = ((regA >> 13) ^ (regA >> 16) ^ (regA >> 17) ^ (regA >> 18)) & 1;
            regA = ((regA << 1) & M19) | fb;
        }
        if (vB == maj) {
            int fb = ((regB >> 20) ^ (regB >> 21)) & 1;
            regB = ((regB << 1) & M22) | fb;
        }
        if (vC == maj) {
            int fb = ((regC >> 7) ^ (regC >> 20) ^ (regC >> 21) ^ (regC >> 22)) & 1;
            regC = ((regC << 1) & M23) | fb;
        }
        ksBits[22] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 23 ------------------------------------------------
    {
        int vA = clkBitA(regA);
        int vB = clkBitB(regB);
        int vC = clkBitC(regC);
        int sum = vA + vB + vC;
        int maj = (sum >= 2);

        if (vA == maj) {
            int fb = ((regA >> 13) ^ (regA >> 16) ^ (regA >> 17) ^ (regA >> 18)) & 1;
            regA = ((regA << 1) & M19) | fb;
        }
        if (vB == maj) {
            int fb = ((regB >> 20) ^ (regB >> 21)) & 1;
            regB = ((regB << 1) & M22) | fb;
        }
        if (vC == maj) {
            int fb = ((regC >> 7) ^ (regC >> 20) ^ (regC >> 21) ^ (regC >> 22)) & 1;
            regC = ((regC << 1) & M23) | fb;
        }
        ksBits[23] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 24 ------------------------------------------------
    {
        int vA = clkBitA(regA);
        int vB = clkBitB(regB);
        int vC = clkBitC(regC);
        int sum = vA + vB + vC;
        int maj = (sum >= 2);

        if (vA == maj) {
            int fb = ((regA >> 13) ^ (regA >> 16) ^ (regA >> 17) ^ (regA >> 18)) & 1;
            regA = ((regA << 1) & M19) | fb;
        }
        if (vB == maj) {
            int fb = ((regB >> 20) ^ (regB >> 21)) & 1;
            regB = ((regB << 1) & M22) | fb;
        }
        if (vC == maj) {
            int fb = ((regC >> 7) ^ (regC >> 20) ^ (regC >> 21) ^ (regC >> 22)) & 1;
            regC = ((regC << 1) & M23) | fb;
        }
        ksBits[24] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 25 ------------------------------------------------
    {
        int vA = clkBitA(regA);
        int vB = clkBitB(regB);
        int vC = clkBitC(regC);
        int sum = vA + vB + vC;
        int maj = (sum >= 2);

        if (vA == maj) {
            int fb = ((regA >> 13) ^ (regA >> 16) ^ (regA >> 17) ^ (regA >> 18)) & 1);
            regA = ((regA << 1) & M19) | fb;
        }
        if (vB == maj) {
            int fb = ((regB >> 20) ^ (regB >> 21)) & 1;
            regB = ((regB << 1) & M22) | fb;
        }
        if (vC == maj) {
            int fb = ((regC >> 7) ^ (regC >> 20) ^ (regC >> 21) ^ (regC >> 22)) & 1;
            regC = ((regC << 1) & M23) | fb;
        }
        ksBits[25] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 26 ------------------------------------------------
    {
        int vA = clkBitA(regA);
        int vB = clkBitB(regB);
        int vC = clkBitC(regC);
        int sum = vA + vB + vC;
        int maj = (sum >= 2);

        if (vA == maj) {
            int fb = ((regA >> 13) ^ (regA >> 16) ^ (regA >> 17) ^ (regA >> 18)) & 1;
            regA = ((regA << 1) & M19) | fb;
        }
        if (vB == maj) {
            int fb = ((regB >> 20) ^ (regB >> 21)) & 1;
            regB = ((regB << 1) & M22) | fb;
        }
        if (vC == maj) {
            int fb = ((regC >> 7) ^ (regC >> 20) ^ (regC >> 21) ^ (regC >> 22)) & 1;
            regC = ((regC << 1) & M23) | fb;
        }
        ksBits[26] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 27 ------------------------------------------------
    {
        int vA = clkBitA(regA);
        int vB = clkBitB(regB);
        int vC = clkBitC(regC);
        int sum = vA + vB + vC;
        int maj = (sum >= 2);

        if (vA == maj) {
            int fb = ((regA >> 13) ^ (regA >> 16) ^ (regA >> 17) ^ (regA >> 18)) & 1;
            regA = ((regA << 1) & M19) | fb;
        }
        if (vB == maj) {
            int fb = ((regB >> 20) ^ (regB >> 21)) & 1;
            regB = ((regB << 1) & M22) | fb;
        }
        if (vC == maj) {
            int fb = ((regC >> 7) ^ (regC >> 20) ^ (regC >> 21) ^ (regC >> 22)) & 1;
            regC = ((regC << 1) & M23) | fb;
        }
        ksBits[27] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 28 ------------------------------------------------
    {
        int vA = clkBitA(regA);
        int vB = clkBitB(regB);
        int vC = clkBitC(regC);
        int sum = vA + vB + vC;
        int maj = (sum >= 2);

        if (vA == maj) {
            int fb = ((regA >> 13) ^ (regA >> 16) ^ (regA >> 17) ^ (regA >> 18)) & 1;
            regA = ((regA << 1) & M19) | fb;
        }
        if (vB == maj) {
            int fb = ((regB >> 20) ^ (regB >> 21)) & 1;
            regB = ((regB << 1) & M22) | fb;
        }
        if (vC == maj) {
            int fb = ((regC >> 7) ^ (regC >> 20) ^ (regC >> 21) ^ (regC >> 22)) & 1;
            regC = ((regC << 1) & M23) | fb;
        }
        ksBits[28] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 29 ------------------------------------------------
    {
        int vA = clkBitA(regA);
        int vB = clkBitB(regB);
        int vC = clkBitC(regC);
        int sum = vA + vB + vC;
        int maj = (sum >= 2);

        if (vA == maj) {
            int fb = ((regA >> 13) ^ (regA >> 16) ^ (regA >> 17) ^ (regA >> 18)) & 1;
            regA = ((regA << 1) & M19) | fb;
        }
        if (vB == maj) {
            int fb = ((regB >> 20) ^ (regB >> 21)) & 1;
            regB = ((regB << 1) & M22) | fb;
        }
        if (vC == maj) {
            int fb = ((regC >> 7) ^ (regC >> 20) ^ (regC >> 21) ^ (regC >> 22)) & 1;
            regC = ((regC << 1) & M23) | fb;
        }
        ksBits[29] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 30 ------------------------------------------------
    {
        int vA = clkBitA(regA);
        int vB = clkBitB(regB);
        int vC = clkBitC(regC);
        int sum = vA + vB + vC;
        int maj = (sum >= 2);

        if (vA == maj) {
            int fb = ((regA >> 13) ^ (regA >> 16) ^ (regA >> 17) ^ (regA >> 18)) & 1;
            regA = ((regA << 1) & M19) | fb;
        }
        if (vB == maj) {
            int fb = ((regB >> 20) ^ (regB >> 21)) & 1;
            regB = ((regB << 1) & M22) | fb;
        }
        if (vC == maj) {
            int fb = ((regC >> 7) ^ (regC >> 20) ^ (regC >> 21) ^ (regC >> 22)) & 1;
            regC = ((regC << 1) & M23) | fb;
        }
        ksBits[30] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 31 ------------------------------------------------
    {
        int vA = clkBitA(regA);
        int vB = clkBitB(regB);
        int vC = clkBitC(regC);
        int sum = vA + vB + vC;
        int maj = (sum >= 2);

        if (vA == maj) {
            int fb = ((regA >> 13) ^ (regA >> 16) ^ (regA >> 17) ^ (regA >> 18)) & 1;
            regA = ((regA << 1) & M19) | fb;
        }
        if (vB == maj) {
            int fb = ((regB >> 20) ^ (regB >> 21)) & 1;
            regB = ((regB << 1) & M22) | fb;
        }
        if (vC == maj) {
            int fb = ((regC >> 7) ^ (regC >> 20) ^ (regC >> 21) ^ (regC >> 22)) & 1;
            regC = ((regC << 1) & M23) | fb;
        }
        ksBits[31] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ------------------------------------------------------------
    // 7.  Assemble bits into a 32‑bit word for pretty printing
    // ------------------------------------------------------------
    int keystream = 0;
    for (int i = 0; i < 32; ++i) {
        keystream = (keystream << 1) | ksBits[i];
    }

    // ------------------------------------------------------------
    // 8.  Output
    // ------------------------------------------------------------
    std::cout << "A5/1 keystream (32 bits) = 0x";
    // print as 8‑hex‑digit value without using unsigned
    int nibble[8];
    for (int i = 0; i < 8; ++i) {
        nibble[i] = (keystream >> ((7 - i) * 4)) & 0xF;
    }
    for (int i = 0; i < 8; ++i) {
        int d = nibble[i];
        char c = (d < 10) ? ('0' + d) : ('A' + d - 10);
        std::cout << c;
    }
    std::cout << std::endl;
    return 0;
}
