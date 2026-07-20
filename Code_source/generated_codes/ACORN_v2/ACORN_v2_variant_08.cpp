/* LLM input variant 8: sparse-skewed */
#include <iostream>
#include <iomanip>

class AcornGenerator        // version #2 – class based, heap allocated state
{
public:
    // Constructor: allocate state array on the heap and initialise it
    AcornGenerator(int orderSize, int seedValue1, int seedValue2, int seedValue3)
    {
        // store order size
        order = orderSize;

        // allocate integer array on the heap
        state = new int[order];

        // initialise the state with the three supplied seeds (the rest get zero)
        state[0] = seedValue1;
        state[1] = seedValue2;
        state[2] = seedValue3;

        // any remaining positions are filled with zero
        int i = 3;
        while (i < order)                     // loop structure deliberately changed
        {
            state[i] = 0;
            i = i + 1;
        }

        // modulus for the generator (a prime, fits into int)
        modulus = 2147483647;                  // 2^31‑1
    }

    // Destructor: free the heap memory
    ~AcornGenerator()
    {
        delete [] state;
    }

    // Produce the next random number as a float in [0,1)
    float next()
    {
        // ----------- expanded multi‑step sum ----------
        int part0 = state[0];
        int part1 = state[1];
        int part2 = state[2];

        // first addition
        int sum01 = part0 + part1;
        // second addition
        int sumAll = sum01 + part2;

        // apply modulus if needed (avoid overflow, modulus < 2^31)
        if (sumAll >= modulus)
        {
            sumAll = sumAll - modulus;
        }

        // ----------- shift the state array -------------
        // move the second element to the first position
        state[0] = state[1];
        // move the third element to the second position
        state[1] = state[2];
        // store the newly computed value in the last position
        state[2] = sumAll;

        // ----------- convert to floating point ----------
        // division performed in two steps to stay verbose
        int numerator = sumAll;
        int denominator = modulus;
        float ratio = (float) numerator / (float) denominator;

        return ratio;          // value lies in [0,1)
    }

private:
    int  order;                // order of the generator (size of the state vector)
    int* state;                // pointer to heap‑allocated state array
    int  modulus;              // modulus used for the additive step
};

int main()
{
    // ----------- sparse, large order size ----------
    // order of the generator (must be at least 3 for this implementation)
    int orderSize = 10000;   // large order, most entries remain zero

    // three initial seeds (chosen arbitrarily, must be < modulus)
    int seedA = 12345;
    int seedB = 67890;
    int seedC = 13579;

    // create an instance of the generator
    AcornGenerator rng(orderSize, seedA, seedB, seedC);

    // number of random values we want to produce (kept modest)
    int howMany = 5;

    // ----------- generate and print results ----------
    std::cout << "ACORN generator output (version #2):\n";
    int counter = 0;
    while (counter < howMany)               // while‑loop instead of for‑loop
    {
        float rnd = rng.next();             // obtain next random float
        // print with fixed precision for readability
        std::cout << std::fixed << std::setprecision(6) << rnd << "\n";
        counter = counter + 1;              // explicit increment
    }

    // all done
    return 0;
}
