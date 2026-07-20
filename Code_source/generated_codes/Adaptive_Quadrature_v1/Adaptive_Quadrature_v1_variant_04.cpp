#include <iostream>
#include <vector>
#include <cmath>

/* LLM input variant 4: signed-extremes */

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
//  Main – generates a set of edge‑case heavy inputs,
//  runs the adaptive quadrature and prints the outcomes.
//------------------------------------------------------------
int main()
{
    // Edge‑case intervals: mix negative, zero, and positive values,
    // include the discontinuity at 0, very small intervals, and large spans.
    std::vector<float> leftEnds;
    std::vector<float> rightEnds;
    std::vector<float> tolerances;

    // 1. Tiny interval around zero
    leftEnds.push_back(-1e-7f);
    rightEnds.push_back(1e-7f);
    tolerances.push_back(1e-7f);

    // 2. Negative side up to zero (endpoint at zero)
    leftEnds.push_back(-2.5f);
    rightEnds.push_back(0.0f);
    tolerances.push_back(2e-5f);

    // 3. Zero to positive side (start at zero)
    leftEnds.push_back(0.0f);
    rightEnds.push_back(3.7f);
    tolerances.push_back(2e-5f);

    // 4. Large interval crossing zero
    leftEnds.push_back(-8.0f);
    rightEnds.push_back(8.0f);
    tolerances.push_back(1e-4f);

    // 5. Purely negative far side
    leftEnds.push_back(-100.0f);
    rightEnds.push_back(-50.0f);
    tolerances.push_back(1e-6f);

    // 6. Purely positive far side
    leftEnds.push_back(50.0f);
    rightEnds.push_back(100.0f);
    tolerances.push_back(1e-6f);

    // Process each test case
    int idx = 0;
    while (idx < static_cast<int>(leftEnds.size()))
    {
        float a = leftEnds[idx];
        float b = rightEnds[idx];
        float eps = tolerances[idx];

        std::vector<float> segLengths;
        float integral = adaptiveIntegrate(a, b, eps, segLengths);

        // Output
        std::cout << "Case " << idx + 1 << " : [" << a << ", " << b << "]\n";
        std::cout << "  Tolerance   = " << eps << "\n";
        std::cout << "  Approx. int = " << integral << "\n";
        std::cout << "  Sub‑intervals used = " << static_cast<int>(segLengths.size()) << "\n";
        std::cout << "  Total length covered = " << b - a << "\n\n";

        ++idx;
    }

    return 0;
}
