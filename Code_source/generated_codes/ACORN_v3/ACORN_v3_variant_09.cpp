#include <iostream>
#include <climits>

#define MODULO 2147483647   // 2^31‑1, fits in a signed int

/* LLM input variant 9: medium-deterministic-random */

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
//  Main entry point – runs a few edge‑case heavy test scenarios.
//===================================================================
int main()
{
    // ---------------------------------------------------------------
    //  Test 1: Small order (k = 2) with non‑zero deterministic seeds.
    // ---------------------------------------------------------------
    int orderA = 2;
    int *seedA = makeSeeds(orderA, 12345, 67890, 13579);
    AcornRng rngA(orderA, seedA);

    std::cout << "Test 1 – order=2, deterministic seeds\n";
    int count = 0;
    while (count < 6)
    {
        float value = rngA.next();
        std::cout << "  " << value << "\n";
        count = count + 1;
    }

    // ---------------------------------------------------------------
    //  Test 2: Medium order (k = 7) with a mixed set of extreme values.
    // ---------------------------------------------------------------
    int orderB = 7;
    int mixedSeeds[7] = {
        987654321 % MODULO,
        123456789 % MODULO,
        -987654321,
        0,
        MODULO / 2,
        -123456789,
        INT_MIN
    };
    AcornRng rngB(orderB, mixedSeeds);

    std::cout << "\nTest 2 – order=7, mixed extreme seeds\n";
    count = 0;
    while (count < 12)
    {
        float value = rngB.next();
        std::cout << "  " << value << "\n";
        // intentionally advance counter by 3 each iteration
        count = count + 3;
        // generate a supplemental value when we haven't reached the limit yet
        if (count < 12)
        {
            value = rngB.next();
            std::cout << "  " << value << "\n";
            count = count + 1; // compensate for the extra generation
        }
    }

    // ---------------------------------------------------------------
    //  Test 3: Near‑maximum order (k = 15) with a patterned seed set.
    // ---------------------------------------------------------------
    int orderC = 15;
    int *seedC = makeSeeds(orderC, MODULO / 3, MODULO / 7, MODULO / 11);
    AcornRng rngC(orderC, seedC);

    std::cout << "\nTest 3 – order=15, patterned large seeds\n";
    count = 0;
    do
    {
        float value = rngC.next();
        std::cout << "  " << value << "\n";
        count = count + 1;
    } while (count < 10);

    return 0;
}
