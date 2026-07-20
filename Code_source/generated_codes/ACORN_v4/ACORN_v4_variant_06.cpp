#include <iostream>

#define MODMASK 0x7fffffff   // keep values in 31‑bit range

/* LLM input variant 6: ordered-structured */

// ------------------------------------------------------------
// ACORN PRNG – version #4 (class based, stack arrays, verbose)
// ------------------------------------------------------------
class ACORN {
    int order_;          // order of the generator
    int state_[10];      // internal state (max order 10)

public:
    // Constructor: deterministic initialization with a symmetric descending pattern
    ACORN(int ord) {
        // store order
        order_ = ord;

        // initialise each element with a descending deterministic pattern
        int idx = 0;
        while (idx < order_) {
            int base   = order_ - idx;          // descending seed value
            int masked = base & MODMASK;       // apply mask
            state_[idx] = masked;
            idx = idx + 1;
        }
    }

    // Produce the next random integer
    int next() {
        // -------- step 1 : update first component ----------
        int inc      = 1;                     // increment value
        int old0     = state_[0];
        int sum0     = old0 + inc;
        int new0     = sum0 & MODMASK;        // wrap around
        state_[0]    = new0;

        // -------- step 2 : cascade updates for the rest ----------
        int i = 1;
        while (i < order_) {
            // fetch previous component (already updated)
            int prev_val = state_[i - 1];

            // fetch current component
            int cur_val  = state_[i];

            // add them
            int temp_sum = cur_val + prev_val;

            // apply mask to stay in range
            int new_val  = temp_sum & MODMASK;

            // store back
            state_[i] = new_val;

            // next index
            i = i + 1;
        }

        // -------- output ----------
        int out = state_[order_ - 1];
        return out;
    }
};

// ------------------------------------------------------------
// Main – generate deterministic test vector
// ------------------------------------------------------------
int main() {
    // deterministic order (use maximum order for a highly structured state)
    int chosen_order = 10;

    // instantiate generator
    ACORN generator(chosen_order);

    // produce and print ten values
    int produced = 0;
    while (produced < 10) {
        // obtain next value
        int raw = generator.next();

        // print it
        std::cout << raw << std::endl;

        // count
        produced = produced + 1;
    }

    return 0;
}
