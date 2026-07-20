#include <cstdio>

/* LLM input variant 10: large-safe-stress */

int main() {
    // ----- parameters -----
    int order = 5;                 // order of the ACORN generator
    int total = 20000;             // how many random numbers to produce (large but safe)
    int modulus = 1000000007;      // prime modulus to keep values in range

    // ----- seed array (stack allocated) -----
    int state[5];
    state[0] = 123456789;
    state[1] = 987654321;
    state[2] = 192837465;
    state[3] = 564738291;
    state[4] = 102030405;

    // ----- container for the output -----
    int result[20000];

    // ----- generation loop -----
    for (int idx = 0; idx < total; ++idx) {
        // break the sum into several explicit steps
        int a0 = state[0];
        int a1 = state[1];
        int a2 = state[2];
        int a3 = state[3];
        int a4 = state[4];

        int partial = a0;
        partial = partial + a1;
        partial = partial + a2;
        partial = partial + a3;
        partial = partial + a4;

        // keep the value inside the modulus
        int next = partial % modulus;

        // store the generated number
        result[idx] = next;

        // shift the state left and insert the new value at the end
        int t0 = state[1];
        int t1 = state[2];
        int t2 = state[3];
        int t3 = state[4];

        state[0] = t0;
        state[1] = t1;
        state[2] = t2;
        state[3] = t3;
        state[4] = next;
    }

    // ----- print the generated sequence -----
    for (int i = 0; i < total; ++i) {
        // promote to float only for printing
        float outVal = result[i];
        printf("%f\n", outVal);
    }

    return 0;
}
