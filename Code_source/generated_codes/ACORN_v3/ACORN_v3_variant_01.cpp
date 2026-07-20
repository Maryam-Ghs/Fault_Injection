/* LLM input variant 1: minimal-boundary */
#include <iostream>
#include <climits>

#define MODULO 2147483647   // 2^31‑1, fits in a signed int

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
//  Main entry point – runs minimal‑boundary test scenarios.
//===================================================================
int main()
{
    // ---------------------------------------------------------------
    //  Test 1: Minimal order (k = 1) with maximal seed (MODULO‑1).
    // ---------------------------------------------------------------
    int orderA = 1;
    int *seedA = makeSeeds(orderA, MODULO - 1, 0, 0);
    AcornRng rngA(orderA, seedA);

    std::cout << "Test 1 – order=1, seed=MODULO-1\n";
    int count = 0;
    while (count < 2)   // generate two values
    {
        float value = rngA.next();
        std::cout << "  " << value << "\n";
        count = count + 1;
    }

    // ---------------------------------------------------------------
    //  Test 2: Minimal non‑trivial order (k = 2) with boundary seeds 0
    //          and MODULO‑1.
    // ---------------------------------------------------------------
    int orderB = 2;
    int *seedB = makeSeeds(orderB, 0, MODULO - 1, 0);
    AcornRng rngB(orderB, seedB);

    std::cout << "\nTest 2 – order=2, seeds=0 and MODULO-1\n";
    count = 0;
    while (count < 2)   // generate two values
    {
        float value = rngB.next();
        std::cout << "  " << value << "\n";
        count = count + 1;
    }

    // ---------------------------------------------------------------
    //  Test 3: Small order (k = 3) with repeating small positive seeds.
    // ---------------------------------------------------------------
    int orderC = 3;
    int *seedC = makeSeeds(orderC, 1, 1, 1);
    AcornRng rngC(orderC, seedC);

    std::cout << "\nTest 3 – order=3, seeds=1,1,1\n";
    count = 0;
    while (count < 2)   // generate two values
    {
        float value = rngC.next();
        std::cout << "  " << value << "\n";
        count = count + 1;
    }

    return 0;
}
