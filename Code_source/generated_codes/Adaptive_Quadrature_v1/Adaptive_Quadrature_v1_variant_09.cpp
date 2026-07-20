#include <iostream>
#include <vector>
#include <cmath>

/* LLM input variant 9: medium-deterministic-random */

//------------------------------------------------------------
//  Function to be integrated – a heavy edge‑case example.
//  It is a step function with a discontinuity at x = 0.
//------------------------------------------------------------
float target(float x)
{
    // piecewise definition
    if (x < 0.0f) return 0.0f;
    return 1.0f;
}

//------------------------------------------------------------
//  Simpson rule on a single interval [l, r].
//  Uses pre‑computed function values to avoid extra calls.
//------------------------------------------------------------
float simpsonStep(float l, float r,
                  float fl, float fm, float fr)
{
    float width = r - l;
    // (width/6) * (fl + 4*fm + fr)
    float term1 = fl + fr;
    float term2 = 4.0f * fm;
    float sum   = term1 + term2;
    float coeff = width / 6.0f;
    return coeff * sum;
}

//------------------------------------------------------------
//  Recursive adaptive routine.
//  Stores the lengths of sub‑intervals that were accepted
//  in the vector 'segments' and accumulates the integral in
//  the reference variable 'total'.
//------------------------------------------------------------
void adaptRecursive(float l, float r,
                    float fl, float fm, float fr,
                    float whole, float eps,
                    std::vector<float>& segments,
                    float& total)
{
    // mid‑points of the two halves
    float m1 = (l + fm) * 0.5f;   // (l + (l+r)/2)/2 = (l + r)/4
    float m2 = (fm + r) * 0.5f;   // ((l+r)/2 + r)/2 = 3*(l+r)/4

    // function values at the new points
    float f1 = target(m1);
    float f2 = target(m2);

    // Simpson estimates on the two halves
    float left  = simpsonStep(l, fm, fl, f1, fm);
    float right = simpsonStep(fm, r, fm, f2, fr);
    float refined = left + right;

    // error estimate (absolute)
    float delta = refined - whole;
    if (std::fabs(delta) <= 15.0f * eps)
    {
        // accept this pair of sub‑intervals
        segments.push_back(r - l);
        total += refined + delta / 15.0f;   // Richardson extrapolation
    }
    else
    {
        // split further, halve the tolerance for each half
        adaptRecursive(l, fm, fl, f1, fm, left,
                       eps * 0.5f, segments, total);
        adaptRecursive(fm, r, fm, f2, fr, right,
                       eps * 0.5f, segments, total);
    }
}

//------------------------------------------------------------
//  Driver that prepares the initial data and launches the
//  adaptive process.
//------------------------------------------------------------
float adaptiveIntegrate(float a, float b,
                        float tolerance,
                        std::vector<float>& segs)
{
    // initial function evaluations
    float fa = target(a);
    float fb = target(b);
    float fm = target(0.5f * (a + b));

    // whole interval Simpson estimate
    float whole = simpsonStep(a, b, fa, fm, fb);

    float result = 0.0f;
    adaptRecursive(a, b, fa, fm, fb,
                   whole, tolerance, segs, result);
    return result;
}

//------------------------------------------------------------
//  Simple deterministic pseudo‑random generator (LCG)
//------------------------------------------------------------
static uint32_t lcg_state = 24681012u;
uint32_t lcg_next()
{
    lcg_state = lcg_state * 1664525u + 1013904223u;
    return lcg_state;
}

// Generate a float in [low, high) using the LCG
float gen_float(float low, float high)
{
    uint32_t r = lcg_next();
    return low + (high - low) * (static_cast<float>(r) / static_cast<float>(UINT32_MAX));
}

//------------------------------------------------------------
//  Main – generates a deterministic set of diverse inputs,
//  runs the adaptive quadrature and prints the outcomes.
//------------------------------------------------------------
int main()
{
    const int TEST_COUNT = 10;        // medium‑sized test suite
    std::vector<float> leftEnds;
    std::vector<float> rightEnds;
    std::vector<float> tolerances;

    // Produce a mixture of intervals: some cross zero, some are far,
    // some are very small, some are large.  Deterministic pseudo‑random.
    for (int i = 0; i < TEST_COUNT; ++i)
    {
        // Width between 0.0005 and 20.0
        float width = gen_float(0.0005f, 20.0f);

        // Center position between -15 and 15
        float center = gen_float(-15.0f, 15.0f);

        float a = center - width * 0.5f;
        float b = center + width * 0.5f;

        // Ensure a < b
        if (a == b) { b = a + 0.001f; }

        leftEnds.push_back(a);
        rightEnds.push_back(b);

        // Tolerance between 1e-7 and 1e-4, scaled pseudo‑randomly
        float baseTol = 1e-7f;
        float scale   = gen_float(1.0f, 1000.0f); // up to 1e-4
        tolerances.push_back(baseTol * scale);
    }

    // Process each test case
    for (int idx = 0; idx < TEST_COUNT; ++idx)
    {
        float a   = leftEnds[idx];
        float b   = rightEnds[idx];
        float eps = tolerances[idx];

        std::vector<float> segLengths;
        float integral = adaptiveIntegrate(a, b, eps, segLengths);

        std::cout << "Case " << idx + 1 << " : [" << a << ", " << b << "]\n";
        std::cout << "  Tolerance   = " << eps << "\n";
        std::cout << "  Approx. int = " << integral << "\n";
        std::cout << "  Sub‑intervals used = " << static_cast<int>(segLengths.size()) << "\n";
        std::cout << "  Total length covered = " << b - a << "\n\n";
    }

    return 0;
}
