#include <iostream>
#include <climits>

#define MODULO 2147483647   // 2^31‑1, fits in a signed int

/* LLM input variant 10: large-safe-stress */

//===================================================================
//  Class based implementation of the ACORN (Additive Congruential)
//  Random Number Generator.  This version is deliberately verbose,
//  uses stack‑allocated arrays, reorders arithmetic where possible
//  and works only with int and float types.
//===================================================================
class AcornRng
{
public:
    // ----------------------------------------------------------------
    //  Constructor:  order (k) and an array of k initial seeds.
    //  All parameters are stored on the stack – no dynamic memory.
    // ----------------------------------------------------------------
    AcornRng(int order, int *seedArray)
    {
        // store order (k) in a member variable
        k = order;

        // copy the seed values into the internal state array
        // (the state holds the last k generated values)
        int i = 0;
        while (i < k)
        {
            // enforce that every seed lies in [0, MODULO‑1]
            int raw = seedArray[i];
            int reduced = raw % MODULO;
            if (reduced < 0) reduced += MODULO;   // make it non‑negative
            state[i] = reduced;
            i = i + 1;          // loop variable increment written verbosely
        }

        // initialise the position pointer to the first element
        pos = 0;
    }

    // ----------------------------------------------------------------
    //  Produce the next random number in the sequence as a float in
    //  the interval [0,1).  The internal integer state is updated
    //  according to the ACORN recurrence:
    //      x_n = (x_{n-1} + x_{n-2} + … + x_{n-k}) mod MODULO
    //  The arithmetic is deliberately reordered.
    // ----------------------------------------------------------------
    float next()
    {
        // accumulate the sum of the last k values, applying modulo
        // after each addition to stay inside the int range.
        int accumulator = 0;
        int j = 0;
        while (j < k)
        {
            // reorder: add then modulo, instead of summing all first
            int temp = accumulator + state[(pos + j) % k];
            accumulator = temp % MODULO;
            j = j + 1;
        }

        // store the new value into the circular buffer, overwriting
        // the oldest entry.
        state[pos] = accumulator;

        // advance the circular index in a verbose way
        pos = pos + 1;
        if (pos >= k) pos = pos - k;   // wrap‑around without using %
        
        // convert the integer to a float in [0,1)
        // (float division is allowed)
        float result = (float)accumulator / (float)MODULO;
        return result;
    }

private:
    // ----------------------------------------------------------------
    //  Order of the generator (k) – how many previous values are used.
    // ----------------------------------------------------------------
    int k;

    // ----------------------------------------------------------------
    //  Circular buffer holding the last k generated integers.
    //  The array lives on the stack because its size is fixed at compile‑time.
    // ----------------------------------------------------------------
    int state[16];   // maximum supported order is 16 for this demo

    // ----------------------------------------------------------------
    //  Current insertion position inside the circular buffer.
    // ----------------------------------------------------------------
    int pos;
};

//===================================================================
//  Helper: generate a small integer array on the stack.
//  The function returns a pointer to the first element; the caller
//  must not use it after the function returns (but in this program
//  the lifetime is sufficient because we call it immediately).
//===================================================================
int* makeSeeds(int order, int first, int second, int third)
{
    // static storage gives us stack‑like lifetime for this demo
    static int buffer[16];
    int i = 0;
    while (i < order)
    {
        // rotate through the three supplied values
        if (i % 3 == 0) buffer[i] = first;
        else if (i % 3 == 1) buffer[i] = second;
        else buffer[i] = third;
        i = i + 1;
    }
    return buffer;
}

//===================================================================
//  Main entry point – runs a large‑scale stress test while remaining safe.
//===================================================================
int main()
{
    // ---------------------------------------------------------------
    //  Stress Test: Maximum supported order (k = 16) with large,
    //  alternating seeds near MODULO.
    // ---------------------------------------------------------------
    const int order = 16;
    int *seeds = makeSeeds(order, MODULO - 1, MODULO - 2, MODULO - 3);
    AcornRng rng(order, seeds);

    std::cout << "Stress Test – order=16, large alternating seeds\n";

    // Generate a substantial number of outputs (e.g., 1024) to stress the
    // generator without exceeding reasonable runtime or memory.
    const int iterations = 1024;
    int i = 0;
    while (i < iterations)
    {
        float value = rng.next();
        // Print every 64th value to keep output manageable while still
        // demonstrating progression.
        if ((i % 64) == 0)
        {
            std::cout << "  [" << i << "] " << value << "\n";
        }
        i = i + 1;
    }

    return 0;
}
