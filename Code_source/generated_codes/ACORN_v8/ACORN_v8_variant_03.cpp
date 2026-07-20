#include <cstdio>
#include <cstdlib>

/* LLM input variant 3: zeros-and-ones */

int main() {
    // ----- generate a medium‑sized deterministic integer array (the “input”) -----
    int sz = 200;                     // medium size
    int *raw = new int[sz];
    for (int i = 0; i < sz; ++i) raw[i] = (i % 2 == 0) ? 0 : 999;   // alternating 0 and 999 (≈0 and ≈1 after normalisation)

    // ----- ACORN parameters ------------------------------------------------
    int order = 5;                     // order of the generator
    int outCount = 1000;               // how many random numbers to produce

    // ----- initialise the seed vector from the deterministic input ----------------
    float *seed = new float[order];
    for (int i = 0; i < order; ++i) {
        seed[i] = (float)raw[i] / 1000.0f;   // normalise to [0,1)
    }
    delete[] raw;                    // input no longer needed

    // ----- storage for generated numbers ------------------------------------
    float *gen = new float[outCount];

    // ----- core ACORN loop (everything inside main) ------------------------
    int pos = 0;                     // position in output array
    while (pos < outCount) {
        // compute new value: sum of first and last seed, wrap in [0,1)
        float nxt = seed[0] + seed[order - 1];
        nxt -= (nxt >= 1.0f) ? 1.0f : 0.0f;   // branch‑minimised wrap

        // shift the seed vector right‑to‑left (no extra temporaries)
        int j = order - 1;
        while (j > 0) { seed[j] = seed[j - 1]; --j; }

        seed[0] = nxt;               // insert new seed at front
        gen[pos++] = nxt;            // store output
    }

    // ----- output a sample of the generated sequence ------------------------
    for (int i = 0; i < 10; ++i) {
        printf("%f ", gen[i]);
    }
    printf("\n");

    // ----- clean up ---------------------------------------------------------
    delete[] seed;
    delete[] gen;
    return 0;
}
