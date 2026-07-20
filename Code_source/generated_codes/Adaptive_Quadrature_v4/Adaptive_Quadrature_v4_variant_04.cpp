/* LLM input variant 4: signed-extremes */
#include <iostream>
#include <cmath>

//------------------------------------------------------------
// Simple adaptive Simpson integrator – version #4
// Uses only float, stack allocation and manual loop unrolling
//------------------------------------------------------------

#define STACKSIZE 1024

class AdaptiveIntegrator {
public:
    // function pointer type for integrand
    typedef float (*FuncPtr)(float);

    // integrate FuncPtr f over [x0, x1] with absolute tolerance eps
    float integrate(FuncPtr f, float x0, float x1, float eps) {
        // stack element
        struct Frame {
            float a;      // left endpoint
            float b;      // right endpoint
            float fa;     // f(a)
            float fb;     // f(b)
            float fm;     // f((a+b)/2)
            float whole;  // Simpson estimate on [a,b]
        };

        // static stack on the call frame
        Frame stack[STACKSIZE];
        int top = 0;

        // initial interval
        float fL = f(x0);
        float fR = f(x1);
        float mid = (x0 + x1) * 0.5f;
        float fM = f(mid);
        float whole = (x1 - x0) * (fL + 4.0f * fM + fR) * (1.0f / 6.0f);
        stack[top++] = { x0, x1, fL, fR, fM, whole };

        float total = 0.0f;

        // main processing loop – manual unrolling of sub‑interval work
        while (top > 0) {
            // pop current frame
            Frame cur = stack[--top];
            float a = cur.a;
            float b = cur.b;
            float fa = cur.fa;
            float fb = cur.fb;
            float fm = cur.fm;

            // mid‑points of halves
            float m  = (a + b) * 0.5f;
            float lm = (a + m) * 0.5f;   // left‑half midpoint
            float rm = (m + b) * 0.5f;   // right‑half midpoint

            // evaluate integrand at new points (unrolled)
            float flm = f(lm);
            float frm = f(rm);

            // Simpson estimates on the two halves (unrolled)
            float leftArea  = (m - a) * (fa + 4.0f * flm + fm) * (1.0f / 6.0f);
            float rightArea = (b - m) * (fm + 4.0f * frm + fb) * (1.0f / 6.0f);

            // error estimate (standard Simpson error scaling)
            float err = (leftArea + rightArea - cur.whole) * (1.0f / 15.0f);

            // accept or subdivide
            if (err <= eps) {
                total += leftArea + rightArea;
            } else {
                // push right half then left half (stack LIFO order)
                stack[top++] = { m, b, fm, fb, frm, rightArea };
                stack[top++] = { a, m, fa, fm, flm, leftArea };
            }
        }
        return total;
    }
};

//------------------------------------------------------------
// Test integrands (float versions only)
//------------------------------------------------------------
float func_sine(float x) {
    // std::sin returns double – cast to float
    return static_cast<float>(std::sin(x));
}

float func_square(float x) {
    return x * x;
}

//------------------------------------------------------------
// Deterministic test vectors – no external input
//------------------------------------------------------------
int main() {
    AdaptiveIntegrator solver;

    // Test 1: ∫_{-π}^{π} sin(x) dx = 0
    float a1 = -static_cast<float>(M_PI);
    float b1 =  static_cast<float>(M_PI);
    float tol1 = 1e-5f;
    float res1 = solver.integrate(func_sine, a1, b1, tol1);
    std::cout << "Integral sin(x) from -pi to pi ≈ " << res1 << "\n";

    // Test 2: ∫_{-1}^{1} x^2 dx = 2/3 ≈ 0.6666667
    float a2 = -1.0f;
    float b2 =  1.0f;
    float tol2 = 1e-5f;
    float res2 = solver.integrate(func_square, a2, b2, tol2);
    std::cout << "Integral x^2 from -1 to 1 ≈ " << res2 << "\n";

    // Test 3: Zero‑length interval ∫_{0}^{0} sin(x) dx = 0
    float a3 = 0.0f;
    float b3 = 0.0f;
    float tol3 = 1e-5f;
    float res3 = solver.integrate(func_sine, a3, b3, tol3);
    std::cout << "Integral sin(x) from 0 to 0 ≈ " << res3 << "\n";

    // Test 4: ∫_{-10}^{0} x^2 dx = (10^3)/3 ≈ 333.33333
    float a4 = -10.0f;
    float b4 =  0.0f;
    float tol4 = 1e-4f;
    float res4 = solver.integrate(func_square, a4, b4, tol4);
    std::cout << "Integral x^2 from -10 to 0 ≈ " << res4 << "\n";

    return 0;
}
