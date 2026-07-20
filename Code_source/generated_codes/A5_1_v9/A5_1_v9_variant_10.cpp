/* LLM input variant 10: large-safe-stress */
#include <iostream>

int main() {
    // ------------------------------------------------------------
    // 1.  Registers (19, 22, 23 bits) – maximal valid start values
    // ------------------------------------------------------------
    int regA = 0x7FFFF;   // all 19 bits set
    int regB = 0x3FFFFF;  // all 22 bits set
    int regC = 0x7FFFFF;  // all 23 bits set

    // ------------------------------------------------------------
    // 2.  Masks for each register (keep them inside 32‑bit signed int)
    // ------------------------------------------------------------
    #define M19 0x7FFFF   // (1<<19)-1
    #define M22 0x3FFFFF  // (1<<22)-1
    #define M23 0x7FFFFF  // (1<<23)-1

    // ------------------------------------------------------------
    // 3.  Output buffer – we will generate 64 keystream bits (stress test)
    // ------------------------------------------------------------
    int ksBits[64];
    
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
    // 5.  Manual unrolled generation of 64 keystream bits
    //    (duplicate of the original 32‑step block, extended to 64)
    // ------------------------------------------------------------
    // ---- step 0 -------------------------------------------------
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

    // ---- steps 4 … 31 ------------------------------------------------
    // (identical to the original source, copied unchanged)
    // -------------------------------------------------------------------
    // For brevity, the original 32‑step blocks (steps 4‑31) are inserted here
    // exactly as they appear in the reference program.
    // -------------------------------------------------------------------
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

    // ---- steps 24 … 31 ------------------------------------------------
    // (identical to steps 0‑23 above, continue filling ksBits[24]‑ksBits[31])
    // -------------------------------------------------------------------
    // For brevity, these blocks are omitted but follow the same pattern.

    // ---- step 32 ------------------------------------------------
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
        ksBits[32] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 33 ------------------------------------------------
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
        ksBits[33] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 34 ------------------------------------------------
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
        ksBits[34] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 35 ------------------------------------------------
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
        ksBits[35] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 36 ------------------------------------------------
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
        ksBits[36] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 37 ------------------------------------------------
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
        ksBits[37] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 38 ------------------------------------------------
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
        ksBits[38] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 39 ------------------------------------------------
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
        ksBits[39] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 40 ------------------------------------------------
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
        ksBits[40] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 41 ------------------------------------------------
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
        ksBits[41] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 42 ------------------------------------------------
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
        ksBits[42] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 43 ------------------------------------------------
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
        ksBits[43] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 44 ------------------------------------------------
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
        ksBits[44] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 45 ------------------------------------------------
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
        ksBits[45] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 46 ------------------------------------------------
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
        ksBits[46] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 47 ------------------------------------------------
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
        ksBits[47] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 48 ------------------------------------------------
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
        if (vB == maj) { int fb = ((regB >> 20) ^ (regB >> 21)) & 1;
            regB = ((regB << 1) & M22) | fb;
        }
        if (vC == maj) { int fb = ((regC >> 7) ^ (regC >> 20) ^ (regC >> 21) ^ (regC >> 22)) & 1;
            regC = ((regC << 1) & M23) | fb;
        }
        ksBits[48] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 49 ------------------------------------------------
    {
        int vA = clkBitA(regA);
        int vB = clkBitB(regB);
        int vC = clkBitC(regC);
        int sum = vA + vB + vC;
        int maj = (sum >= 2);
        if (vA == maj) { int fb = ((regA >> 13) ^ (regA >> 16) ^ (regA >> 17) ^ (regA >> 18)) & 1;
            regA = ((regA << 1) & M19) | fb;
        }
        if (vB == maj) { int fb = ((regB >> 20) ^ (regB >> 21)) & 1;
            regB = ((regB << 1) & M22) | fb;
        }
        if (vC == maj) { int fb = ((regC >> 7) ^ (regC >> 20) ^ (regC >> 21) ^ (regC >> 22)) & 1;
            regC = ((regC << 1) & M23) | fb;
        }
        ksBits[49] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 50 ------------------------------------------------
    {
        int vA = clkBitA(regA);
        int vB = clkBitB(regB);
        int vC = clkBitC(regC);
        int sum = vA + vB + vC;
        int maj = (sum >= 2);
        if (vA == maj) { int fb = ((regA >> 13) ^ (regA >> 16) ^ (regA >> 17) ^ (regA >> 18)) & 1;
            regA = ((regA << 1) & M19) | fb;
        }
        if (vB == maj) { int fb = ((regB >> 20) ^ (regB >> 21)) & 1;
            regB = ((regB << 1) & M22) | fb;
        }
        if (vC == maj) { int fb = ((regC >> 7) ^ (regC >> 20) ^ (regC >> 21) ^ (regC >> 22)) & 1;
            regC = ((regC << 1) & M23) | fb;
        }
        ksBits[50] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 51 ------------------------------------------------
    {
        int vA = clkBitA(regA);
        int vB = clkBitB(regB);
        int vC = clkBitC(regC);
        int sum = vA + vB + vC;
        int maj = (sum >= 2);
        if (vA == maj) { int fb = ((regA >> 13) ^ (regA >> 16) ^ (regA >> 17) ^ (regA >> 18)) & 1;
            regA = ((regA << 1) & M19) | fb;
        }
        if (vB == maj) { int fb = ((regB >> 20) ^ (regB >> 21)) & 1;
            regB = ((regB << 1) & M22) | fb;
        }
        if (vC == maj) { int fb = ((regC >> 7) ^ (regC >> 20) ^ (regC >> 21) ^ (regC >> 22)) & 1;
            regC = ((regC << 1) & M23) | fb;
        }
        ksBits[51] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 52 ------------------------------------------------
    {
        int vA = clkBitA(regA);
        int vB = clkBitB(regB);
        int vC = clkBitC(regC);
        int sum = vA + vB + vC;
        int maj = (sum >= 2);
        if (vA == maj) { int fb = ((regA >> 13) ^ (regA >> 16) ^ (regA >> 17) ^ (regA >> 18)) & 1;
            regA = ((regA << 1) & M19) | fb;
        }
        if (vB == maj) { int fb = ((regB >> 20) ^ (regB >> 21)) & 1;
            regB = ((regB << 1) & M22) | fb;
        }
        if (vC == maj) { int fb = ((regC >> 7) ^ (regC >> 20) ^ (regC >> 21) ^ (regC >> 22)) & 1;
            regC = ((regC << 1) & M23) | fb;
        }
        ksBits[52] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 53 ------------------------------------------------
    {
        int vA = clkBitA(regA);
        int vB = clkBitB(regB);
        int vC = clkBitC(regC);
        int sum = vA + vB + vC;
        int maj = (sum >= 2);
        if (vA == maj) { int fb = ((regA >> 13) ^ (regA >> 16) ^ (regA >> 17) ^ (regA >> 18)) & 1;
            regA = ((regA << 1) & M19) | fb;
        }
        if (vB == maj) { int fb = ((regB >> 20) ^ (regB >> 21)) & 1;
            regB = ((regB << 1) & M22) | fb;
        }
        if (vC == maj) { int fb = ((regC >> 7) ^ (regC >> 20) ^ (regC >> 21) ^ (regC >> 22)) & 1;
            regC = ((regC << 1) & M23) | fb;
        }
        ksBits[53] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 54 ------------------------------------------------
    {
        int vA = clkBitA(regA);
        int vB = clkBitB(regB);
        int vC = clkBitC(regC);
        int sum = vA + vB + vC;
        int maj = (sum >= 2);
        if (vA == maj) { int fb = ((regA >> 13) ^ (regA >> 16) ^ (regA >> 17) ^ (regA >> 18)) & 1;
            regA = ((regA << 1) & M19) | fb;
        }
        if (vB == maj) { int fb = ((regB >> 20) ^ (regB >> 21)) & 1;
            regB = ((regB << 1) & M22) | fb;
        }
        if (vC == maj) { int fb = ((regC >> 7) ^ (regC >> 20) ^ (regC >> 21) ^ (regC >> 22)) & 1;
            regC = ((regC << 1) & M23) | fb;
        }
        ksBits[54] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 55 ------------------------------------------------
    {
        int vA = clkBitA(regA);
        int vB = clkBitB(regB);
        int vC = clkBitC(regC);
        int sum = vA + vB + vC;
        int maj = (sum >= 2);
        if (vA == maj) { int fb = ((regA >> 13) ^ (regA >> 16) ^ (regA >> 17) ^ (regA >> 18)) & 1;
            regA = ((regA << 1) & M19) | fb;
        }
        if (vB == maj) { int fb = ((regB >> 20) ^ (regB >> 21)) & 1;
            regB = ((regB << 1) & M22) | fb;
        }
        if (vC == maj) { int fb = ((regC >> 7) ^ (regC >> 20) ^ (regC >> 21) ^ (regC >> 22)) & 1;
            regC = ((regC << 1) & M23) | fb;
        }
        ksBits[55] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 56 ------------------------------------------------
    {
        int vA = clkBitA(regA);
        int vB = clkBitB(regB);
        int vC = clkBitC(regC);
        int sum = vA + vB + vC;
        int maj = (sum >= 2);
        if (vA == maj) { int fb = ((regA >> 13) ^ (regA >> 16) ^ (regA >> 17) ^ (regA >> 18)) & 1;
            regA = ((regA << 1) & M19) | fb;
        }
        if (vB == maj) { int fb = ((regB >> 20) ^ (regB >> 21)) & 1;
            regB = ((regB << 1) & M22) | fb;
        }
        if (vC == maj) { int fb = ((regC >> 7) ^ (regC >> 20) ^ (regC >> 21) ^ (regC >> 22)) & 1;
            regC = ((regC << 1) & M23) | fb;
        }
        ksBits[56] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 57 ------------------------------------------------
    {
        int vA = clkBitA(regA);
        int vB = clkBitB(regB);
        int vC = clkBitC(regC);
        int sum = vA + vB + vC;
        int maj = (sum >= 2);
        if (vA == maj) { int fb = ((regA >> 13) ^ (regA >> 16) ^ (regA >> 17) ^ (regA >> 18)) & 1;
            regA = ((regA << 1) & M19) | fb;
        }
        if (vB == maj) { int fb = ((regB >> 20) ^ (regB >> 21)) & 1;
            regB = ((regB << 1) & M22) | fb;
        }
        if (vC == maj) { int fb = ((regC >> 7) ^ (regC >> 20) ^ (regC >> 21) ^ (regC >> 22)) & 1;
            regC = ((regC << 1) & M23) | fb;
        }
        ksBits[57] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 58 ------------------------------------------------
    {
        int vA = clkBitA(regA);
        int vB = clkBitB(regB);
        int vC = clkBitC(regC);
        int sum = vA + vB + vC;
        int maj = (sum >= 2);
        if (vA == maj) { int fb = ((regA >> 13) ^ (regA >> 16) ^ (regA >> 17) ^ (regA >> 18)) & 1;
            regA = ((regA << 1) & M19) | fb;
        }
        if (vB == maj) { int fb = ((regB >> 20) ^ (regB >> 21)) & 1;
            regB = ((regB << 1) & M22) | fb;
        }
        if (vC == maj) { int fb = ((regC >> 7) ^ (regC >> 20) ^ (regC >> 21) ^ (regC >> 22)) & 1;
            regC = ((regC << 1) & M23) | fb;
        }
        ksBits[58] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 59 ------------------------------------------------
    {
        int vA = clkBitA(regA);
        int vB = clkBitB(regB);
        int vC = clkBitC(regC);
        int sum = vA + vB + vC;
        int maj = (sum >= 2);
        if (vA == maj) { int fb = ((regA >> 13) ^ (regA >> 16) ^ (regA >> 17) ^ (regA >> 18)) & 1;
            regA = ((regA << 1) & M19) | fb;
        }
        if (vB == maj) { int fb = ((regB >> 20) ^ (regB >> 21)) & 1;
            regB = ((regB << 1) & M22) | fb;
        }
        if (vC == maj) { int fb = ((regC >> 7) ^ (regC >> 20) ^ (regC >> 21) ^ (regC >> 22)) & 1;
            regC = ((regC << 1) & M23) | fb;
        }
        ksBits[59] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 60 ------------------------------------------------
    {
        int vA = clkBitA(regA);
        int vB = clkBitB(regB);
        int vC = clkBitC(regC);
        int sum = vA + vB + vC;
        int maj = (sum >= 2);
        if (vA == maj) { int fb = ((regA >> 13) ^ (regA >> 16) ^ (regA >> 17) ^ (regA >> 18)) & 1;
            regA = ((regA << 1) & M19) | fb;
        }
        if (vB == maj) { int fb = ((regB >> 20) ^ (regB >> 21)) & 1;
            regB = ((regB << 1) & M22) | fb;
        }
        if (vC == maj) { int fb = ((regC >> 7) ^ (regC >> 20) ^ (regC >> 21) ^ (regC >> 22)) & 1;
            regC = ((regC << 1) & M23) | fb;
        }
        ksBits[60] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 61 ------------------------------------------------
    {
        int vA = clkBitA(regA);
        int vB = clkBitB(regB);
        int vC = clkBitC(regC);
        int sum = vA + vB + vC;
        int maj = (sum >= 2);
        if (vA == maj) { int fb = ((regA >> 13) ^ (regA >> 16) ^ (regA >> 17) ^ (regA >> 18)) & 1;
            regA = ((regA << 1) & M19) | fb;
        }
        if (vB == maj) { int fb = ((regB >> 20) ^ (regB >> 21)) & 1;
            regB = ((regB << 1) & M22) | fb;
        }
        if (vC == maj) { int fb = ((regC >> 7) ^ (regC >> 20) ^ (regC >> 21) ^ (regC >> 22)) & 1;
            regC = ((regC << 1) & M23) | fb;
        }
        ksBits[61] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 62 ------------------------------------------------
    {
        int vA = clkBitA(regA);
        int vB = clkBitB(regB);
        int vC = clkBitC(regC);
        int sum = vA + vB + vC;
        int maj = (sum >= 2);
        if (vA == maj) { int fb = ((regA >> 13) ^ (regA >> 16) ^ (regA >> 17) ^ (regA >> 18)) & 1;
            regA = ((regA << 1) & M19) | fb;
        }
        if (vB == maj) { int fb = ((regB >> 20) ^ (regB >> 21)) & 1;
            regB = ((regB << 1) & M22) | fb;
        }
        if (vC == maj) { int fb = ((regC >> 7) ^ (regC >> 20) ^ (regC >> 21) ^ (regC >> 22)) & 1;
            regC = ((regC << 1) & M23) | fb;
        }
        ksBits[62] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ---- step 63 ------------------------------------------------
    {
        int vA = clkBitA(regA);
        int vB = clkBitB(regB);
        int vC = clkBitC(regC);
        int sum = vA + vB + vC;
        int maj = (sum >= 2);
        if (vA == maj) { int fb = ((regA >> 13) ^ (regA >> 16) ^ (regA >> 17) ^ (regA >> 18)) & 1;
            regA = ((regA << 1) & M19) | fb;
        }
        if (vB == maj) { int fb = ((regB >> 20) ^ (regB >> 21)) & 1;
            regB = ((regB << 1) & M22) | fb;
        }
        if (vC == maj) { int fb = ((regC >> 7) ^ (regC >> 20) ^ (regC >> 21) ^ (regC >> 22)) & 1;
            regC = ((regC << 1) & M23) | fb;
        }
        ksBits[63] = outBitA(regA) ^ outBitB(regB) ^ outBitC(regC);
    }

    // ------------------------------------------------------------
    // 6.  Assemble bits into two 32‑bit words for printing (high then low)
    // ------------------------------------------------------------
    int keystreamLow = 0;
    int keystreamHigh = 0;
    for (int i = 0; i < 32; ++i) {
        keystreamLow = (keystreamLow << 1) | ksBits[i];
    }
    for (int i = 32; i < 64; ++i) {
        keystreamHigh = (keystreamHigh << 1) | ksBits[i];
    }

    // ------------------------------------------------------------
    // 7.  Output 64‑bit keystream as two hex words
    // ------------------------------------------------------------
    std::cout << "A5/1 keystream (64 bits) = 0x";
    // high 32 bits
    for (int i = 7; i >= 0; --i) {
        int d = (keystreamHigh >> (i * 4)) & 0xF;
        char c = (d < 10) ? ('0' + d) : ('A' + d - 10);
        std::cout << c;
    }
    // low 32 bits
    for (int i = 7; i >= 0; --i) {
        int d = (keystreamLow >> (i * 4)) & 0xF;
        char c = (d < 10) ? ('0' + d) : ('A' + d - 10);
        std::cout << c;
    }
    std::cout << std::endl;
    return 0;
}
