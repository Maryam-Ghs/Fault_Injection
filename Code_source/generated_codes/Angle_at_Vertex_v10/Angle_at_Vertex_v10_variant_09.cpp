// LLM input variant 9: medium-deterministic-random
// Angle at Vertex – version #10
// Everything is placed inside main(), uses only int and float, stack arrays,
// loop‑heavy iterative processing and expanded multi‑step arithmetic.

#include <cstdio>
#include <cstdlib>
#include <cmath>

int main()
{
    // ------------------------------------------------------------
    // 1.  Parameters (changeable – no const, no unsigned, no double)
    // ------------------------------------------------------------
    int pointCount = 1150;               // size of the deterministic point set
    int maxCoord   = 950;                // coordinate range [0, maxCoord)

    // ------------------------------------------------------------
    // 2.  Stack allocation for X and Y coordinates
    // ------------------------------------------------------------
    #define MAXP 2000                    // compile‑time bound for stack arrays
    float xs[MAXP];
    float ys[MAXP];

    // ------------------------------------------------------------
    // 3.  Fill the arrays with deterministic pseudo‑random numbers
    // ------------------------------------------------------------
    unsigned int seed = 123456789u;       // fixed seed for reproducibility
    int i = 0;
    while (i < pointCount)
    {
        // simple linear congruential generator
        seed = seed * 1664525u + 1013904223u;
        xs[i] = static_cast<float>(seed % maxCoord);

        seed = seed * 1664525u + 1013904223u;
        ys[i] = static_cast<float>(seed % maxCoord);
        ++i;
    }

    // ------------------------------------------------------------
    // 4.  Compute the angle at each interior vertex (i.e., for triples
    //     (i‑1, i, i+1)).  The result is stored temporarily and printed.
    // ------------------------------------------------------------
    int idx = 1;                         // start at second point
    while (idx < pointCount - 1)
    {
        // ----- vector from middle point to previous point -----
        float vx1 = xs[idx - 1] - xs[idx];
        float vy1 = ys[idx - 1] - ys[idx];

        // ----- vector from middle point to next point -------
        float vx2 = xs[idx + 1] - xs[idx];
        float vy2 = ys[idx + 1] - ys[idx];

        // ----- squared lengths (expanded, step‑by‑step) -----
        float sqLen1 = vx1 * vx1;
        sqLen1 = sqLen1 + vy1 * vy1;          // add y‑component

        float sqLen2 = vx2 * vx2;
        sqLen2 = sqLen2 + vy2 * vy2;          // add y‑component

        // ----- lengths -------------------------------------------------
        float len1 = sqrtf(sqLen1);
        float len2 = sqrtf(sqLen2);

        // ----- dot product (expanded) ---------------------------------
        float dot = vx1 * vx2;
        dot = dot + vy1 * vy2;                // add y‑component product

        // ----- protect against division by zero (very small lengths) ---
        if (len1 < 1e-6f) len1 = 1e-6f;
        if (len2 < 1e-6f) len2 = 1e-6f;

        // ----- cosine of the angle ------------------------------------
        float cosTheta = dot / (len1 * len2);

        // ----- clamp cosine into valid range for acos -----------------
        if (cosTheta > 1.0f)  cosTheta = 1.0f;
        if (cosTheta < -1.0f) cosTheta = -1.0f;

        // ----- final angle in radians ---------------------------------
        float angleRad = acosf(cosTheta);

        // ----- output – print every 100th angle to keep output short ----
        if ((idx % 100) == 0)
        {
            std::printf("Vertex %d: angle = %.4f radians\n", idx, angleRad);
        }

        ++idx;                                 // next middle vertex
    }

    // ------------------------------------------------------------
    // 5.  End of program
    // ------------------------------------------------------------
    return 0;
}
