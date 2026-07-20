#include <cstdio>

/* LLM input variant 9: medium-deterministic-random */

int main() {
    // ----- parameters -----
    int order = 7;                 // order of the ACORN generator
    int total = 30;                // how many random numbers to produce
    int modulus = 1000003;         // prime modulus to keep values in range

    // ----- seed array (stack allocated) -----
    int state[7];
    state[0] = 31415;
    state[1] = 92653;
    state[2] = 58979;
    state[3] = 32384;
    state[4] = 62643;
    state[5] = 38327;
    state[6] = 95028;

    // ----- container for the output -----
    int result[30];

    // ----- generation loop -----
    for (int idx = 0; idx < total; ++idx) {
        // break the sum into several explicit steps
        int a0 = state[0];
        int a1 = state[1];
        int a2 = state[2];
        int a3 = state[3];
        int a4 = state[4];
        int a5 = state[5];
        int a6 = state[6];

        int partial = a0;
        partial = partial + a1;
        partial = partial + a2;
        partial = partial + a3;
        partial = partial + a4;
        partial = partial + a5;
        partial = partial + a6;

        // keep the value inside the modulus
        int next = partial % modulus;

        // store the generated number
        result[idx] = next;

        // shift the state left and insert the new value at the end
        int t0 = state[1];
        int t1 = state[2];
        int t2 = state[3];
        int t3 = state[4];
        int t4 = state[5];
        int t5 = state[6];

        state[0] = t0;
        state[1] = t1;
        state[2] = t2;
        state[3] = t3;
        state[4] = t4;
        state[5] = t5;
        state[6] = next;
    }

    // ----- print the generated sequence -----
    for (int i = 0; i < total; ++i) {
        // promote to float only for printing
        float outVal = result[i];
        printf("%f\n", outVal);
    }

    return 0;
}
