#include <cstdio>
#include <cstdlib>
#include <cmath>

/* LLM input variant 7: reverse-adversarial */

int main() {
    /* -----------------------------------------------------------
       Adaptive Refinement – version #2
       * Generates a large random array (stack‑allocated)
       * Performs manual loop unrolling
       * Uses expanded multi‑step arithmetic
       * Everything is inside main()
       ----------------------------------------------------------- */

    int baseSize = 2048;                     // initial number of samples
    int maxSize  = baseSize * 2 - 1;         // worst‑case refined size
    float raw[2048];                         // original data (stack)
    float refined[4095];                     // refined data (stack)

    /* ---- generate deterministic adversarial input (alternating extremes) ---- */
    for (int i = 0; i < baseSize; ++i) {
        raw[i] = (i % 2 == 0) ? 1.0f : 0.0f;  // high, low, high, low, ...
    }

    /* ---- adaptive refinement --------------------------------- */
    float limit = 0.12f;                     // error threshold
    int outIdx = 0;                          // write position in refined[]

    /* copy first element */
    refined[outIdx++] = raw[0];

    /*   process four intervals per iteration (manual unrolling)   */
    int i = 0;
    for (; i + 4 < baseSize; i += 4) {
        /* interval 0: raw[i] – raw[i+1] */
        float a0 = raw[i];
        float b0 = raw[i + 1];
        float d0 = a0 - b0;                  // step 1: difference
        float ad0 = d0 < 0.0f ? -d0 : d0;    // step 2: absolute value
        if (ad0 > limit) {
            float m0 = (a0 + b0) * 0.5f;     // step 3: midpoint
            refined[outIdx++] = m0;
        }
        refined[outIdx++] = b0;

        /* interval 1: raw[i+1] – raw[i+2] */
        float a1 = raw[i + 1];
        float b1 = raw[i + 2];
        float d1 = a1 - b1;
        float ad1 = d1 < 0.0f ? -d1 : d1;
        if (ad1 > limit) {
            float m1 = (a1 + b1) * 0.5f;
            refined[outIdx++] = m1;
        }
        refined[outIdx++] = b1;

        /* interval 2: raw[i+2] – raw[i+3] */
        float a2 = raw[i + 2];
        float b2 = raw[i + 3];
        float d2 = a2 - b2;
        float ad2 = d2 < 0.0f ? -d2 : d2;
        if (ad2 > limit) {
            float m2 = (a2 + b2) * 0.5f;
            refined[outIdx++] = m2;
        }
        refined[outIdx++] = b2;

        /* interval 3: raw[i+3] – raw[i+4] */
        float a3 = raw[i + 3];
        float b3 = raw[i + 4];
        float d3 = a3 - b3;
        float ad3 = d3 < 0.0f ? -d3 : d3;
        if (ad3 > limit) {
            float m3 = (a3 + b3) * 0.5f;
            refined[outIdx++] = m3;
        }
        refined[outIdx++] = b3;
    }

    /* ---- handle the tail of the array (remaining intervals) ---- */
    for (; i < baseSize - 1; ++i) {
        float left  = raw[i];
        float right = raw[i + 1];
        float diff  = left - right;
        float absd  = diff < 0.0f ? -diff : diff;
        if (absd > limit) {
            float mid = (left + right) * 0.5f;
            refined[outIdx++] = mid;
        }
        refined[outIdx++] = right;
    }

    /* ---- output some statistics -------------------------------- */
    printf("Base size          : %d\n", baseSize);
    printf("Refined size       : %d\n", outIdx);
    printf("First 10 refined values:\n");
    for (int k = 0; k < 10 && k < outIdx; ++k) {
        printf("  %.6f\n", refined[k]);
    }

    return 0;
}
