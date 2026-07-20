/* LLM input variant 9: medium-deterministic-random */
#include <cstdio>
#include <cmath>

class AdaptiveRK
{
    // parameters (float only, no const)
    float h;          // current step size
    float tGoal;      // integration end time
    float curT;       // current time
    float curY;       // current solution
    float errTol;     // allowed error per step

    // derivative of the test problem y' = -y
    float deriv(float t, float y)
    {
        return -y;
    }

    // single adaptive RK45 step, returns 1 if step accepted
    int rkStep()
    {
        // ----- stage coefficients (Dormand–Prince) -----
        const float a2 = 0.2f, a3 = 0.3f, a4 = 0.8f, a5 = 8.0f/9.0f, a6 = 1.0f;
        const float b21 = 0.2f;
        const float b31 = 3.0f/40.0f, b32 = 9.0f/40.0f;
        const float b41 = 44.0f/45.0f, b42 = -56.0f/15.0f, b43 = 32.0f/9.0f;
        const float b51 = 19372.0f/6561.0f, b52 = -25360.0f/2187.0f,
                    b53 = 64448.0f/6561.0f, b54 = -212.0f/729.0f;
        const float b61 = 9017.0f/3168.0f, b62 = -355.0f/33.0f,
                    b63 = 46732.0f/5247.0f, b64 = 49.0f/176.0f,
                    b65 = -5103.0f/18656.0f;

        const float c1 = 35.0f/384.0f, c3 = 500.0f/1113.0f,
                    c4 = 125.0f/192.0f, c5 = -2187.0f/6784.0f,
                    c6 = 11.0f/84.0f;               // 5th order solution
        const float cs1 = 35.0f/384.0f, cs3 = 500.0f/1113.0f,
                    cs4 = 125.0f/192.0f, cs5 = -2187.0f/6784.0f,
                    cs6 = 11.0f/84.0f, cs7 = 0.0f; // 4th order solution (no k7)

        // ----- stage values stored on stack -----
        float k[6];

        k[0] = deriv(curT, curY);
        k[1] = deriv(curT + a2 * h,
                     curY + h * (b21 * k[0]));
        k[2] = deriv(curT + a3 * h,
                     curY + h * (b31 * k[0] + b32 * k[1]));
        k[3] = deriv(curT + a4 * h,
                     curY + h * (b41 * k[0] + b42 * k[1] + b43 * k[2]));
        k[4] = deriv(curT + a5 * h,
                     curY + h * (b51 * k[0] + b52 * k[1] + b53 * k[2] + b54 * k[3]));
        k[5] = deriv(curT + a6 * h,
                     curY + h * (b61 * k[0] + b62 * k[1] + b63 * k[2] + b64 * k[3] + b65 * k[4]));

        // ----- compute 5th and 4th order estimates -----
        float y5 = curY + h * (c1 * k[0] + c3 * k[2] + c4 * k[3] + c5 * k[4] + c6 * k[5]);
        float y4 = curY + h * (cs1 * k[0] + cs3 * k[2] + cs4 * k[3] + cs5 * k[4] + cs6 * k[5]);

        // ----- error estimate -----
        float err = fabsf(y5 - y4);

        // ----- step acceptance test -----
        if (err <= errTol || err == 0.0f)
        {
            // accept step
            curT += h;
            curY = y5;   // higher order result
            // output the accepted point
            printf("%8.4f %12.6f\n", curT, curY);
        }

        // ----- adapt step size for next iteration -----
        float factor;
        if (err == 0.0f)
            factor = 5.0f;
        else
        {
            // safety factor 0.9, exponent 0.2 (1/(order+1))
            factor = 0.9f * powf(errTol / err, 0.2f);
            if (factor < 0.1f) factor = 0.1f;
            if (factor > 5.0f) factor = 5.0f;
        }
        h = h * factor;
        // avoid stepping beyond the end time
        if (curT + h > tGoal) h = tGoal - curT;
        // enforce a minimal step size
        if (h < 1e-6f) h = 1e-6f;

        // return 1 if we have reached the goal
        return (curT >= tGoal) ? 1 : 0;
    }

public:
    // constructor – all members are set explicitly (no const)
    AdaptiveRK(float startT, float startY, float endT, float initH, float tolerance)
    {
        curT = startT;
        curY = startY;
        tGoal = endT;
        h = initH;
        errTol = tolerance;
    }

    // driver routine
    void run()
    {
        // print header
        printf("   t        y\n");
        printf("%8.4f %12.6f\n", curT, curY);

        // loop until integration finishes
        while (1)
        {
            int done = rkStep();
            if (done) break;
        }
    }
};

int main()
{
    // deterministic test: solve y' = -y, y(0)=e up to t = π
    float startTime = 0.0f;
    float startVal  = 2.718281f;          // e ≈ 2.718281
    float finalTime = 3.141593f;          // π ≈ 3.141593
    float firstStep = 0.33f;              // medium-sized pseudo‑random step
    float tolerance = 5e-6f;              // tighter tolerance

    AdaptiveRK solver(startTime, startVal, finalTime, firstStep, tolerance);
    solver.run();

    return 0;
}
