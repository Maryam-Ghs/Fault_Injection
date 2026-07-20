/* LLM input variant 1: minimal-boundary */
#include <cstdio>

int main() {
    // ----- parameters -----
    const int order = 1;                 // minimal order of the ACORN generator
    const int total = 1;                 // produce a single random number
    const int modulus = 2;               // smallest prime modulus

    // ----- seed array (stack allocated) -----
    int state[order];
    state[0] = 1;                        // minimal non-zero seed

    // ----- container for the output -----
    int result[total];

    // ----- generation loop -----
    for (int idx = 0; idx < total; ++idx) {
        // compute the sum of the state elements
        int partial = 0;
        for (int i = 0; i < order; ++i) {
            partial += state[i];
        }

        // keep the value inside the modulus
        int next = partial % modulus;

        // store the generated number
        result[idx] = next;

        // shift the state left and insert the new value at the end
        for (int i = 0; i < order - 1; ++i) {
            state[i] = state[i + 1];
        }
        state[order - 1] = next;
    }

    // ----- print the generated sequence -----
    for (int i = 0; i < total; ++i) {
        // promote to float only for printing
        float outVal = result[i];
        printf("%f\n", outVal);
    }

    return 0;
}
