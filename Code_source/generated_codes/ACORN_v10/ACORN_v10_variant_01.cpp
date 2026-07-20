/* LLM input variant 1: minimal-boundary */
/*  ACORN RNG – version #10  */
/*  Restrictions: only int/float, no const, no double, no unsigned, no cin  */

#include <iostream>
#include <iomanip>

/*---------------------------------------------------------------*/
/*  Helper: initialize the state vector from a small seed array   */
void init_vec(int *vec, int *seed, int sz)
{
    for (int i = 0; i < sz; ++i)               /* unrolled copy, no branches   */
        vec[i] = seed[i];
}

/*---------------------------------------------------------------*/
/*  Helper: one ACORN step – order = 5, all‑int arithmetic        */
int acorn_step(int *vec, int ord)
{
    /*  fused sum of the whole vector (order fixed to 5)          */
    int s = vec[0] + vec[1] + vec[2] + vec[3] + vec[4];

    /*  shift left and insert new value (branch‑free)            */
    vec[0] = vec[1];
    vec[1] = vec[2];
    vec[2] = vec[3];
    vec[3] = vec[4];
    vec[4] = s;                                 /* new state element */

    return s;                                   /* raw integer output   */
}

/*---------------------------------------------------------------*/
/*  Helper: generate a block of numbers and print them           */
void produce(int *state, int ord, int cnt)
{
    for (int i = 0; i < cnt; ++i)               /* loop over required count */
    {
        int raw = acorn_step(state, ord);      /* one ACORN iteration       */

        /*  map raw integer to a float in [0,1) without double   */
        float out = (raw & 0x7fffffff) * 1.0f / 2147483648.0f;
        std::cout << std::fixed << std::setprecision(6) << out << '\n';
    }
}

/*---------------------------------------------------------------*/
int main()
{
    /*  minimal non‑zero seed arrays (order = 5)                  */
    int seed_a[5] = { 1, 0, 0, 0, 0 };
    int seed_b[5] = { 0, 1, 0, 0, 0 };

    /*  stack‑allocated state vectors                               */
    int state_a[5];
    int state_b[5];

    /*  initialise both generators                                 */
    init_vec(state_a, seed_a, 5);
    init_vec(state_b, seed_b, 5);

    /*  produce 1 number from each generator                        */
    produce(state_a, 5, 1);
    produce(state_b, 5, 1);

    return 0;
}
