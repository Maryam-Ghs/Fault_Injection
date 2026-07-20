#include <cstdio>

/* LLM input variant 2: small-diverse */

int main() {
    // ----- parameters -----
    int order = 4;                 // order of the ACORN generator
    int total = 12;                // how many random numbers to produce
    int modulus = 1009;      // small prime modulus to keep values in range

    // ----- seed array (stack allocated) -----
    int state[4];
    state[0] = 7;
    state[1] = 42;
    state[2] = 123;
    state[3] = 256;

    // ----- container for the output -----
    int result[12];

    // ----- generation loop -----
    for (int idx = 0; idx < total; ++idx) {
        // break the sum into several explicit steps
        int a0 = state[0];
        int a1 = state[1];
        int a2 = state[2];
        int a3 = state[3];

        int partial = a0;
        partial = partial + a1;
        partial = partial + a2;
        partial = partial + a3;

        // keep the value inside the modulus
        int next = partial % modulus;

        // store the generated number
        result[idx] = next;

        // shift the state left and insert the new value at the end
        int t0 = state[1];
        int t1 = state[2];
        int t2 = state[3];

        state[0] = t0;
        state[1] = t1;
        state[2] = t2;
        state[3] = next;
    }

    // ----- print the generated sequence -----
    for (int i = 0; i < total; ++i) {
        // promote to float only for printing
        float outVal = result[i];
        printf("%f\n", outVal);
    }

    return 0;
}
