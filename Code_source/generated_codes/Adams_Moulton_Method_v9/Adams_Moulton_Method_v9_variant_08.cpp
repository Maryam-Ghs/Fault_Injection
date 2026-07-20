#include <iostream>
#include <vector>

/* LLM input variant 8: sparse-skewed */

class AdamsMoulton9 {
public:
    // Simple ODE: dy/dt = -y
    float rhs(float t, float y) {
        return -y;                     // f(t,y)
    }

    // Solve using 2‑step Adams‑Moulton (trapezoidal) with Adams‑Bashforth predictor
    void solve(float dt, int nSteps, float yStart) {
        std::vector<float> timeVec;
        std::vector<float> solVec;

        timeVec.reserve(nSteps + 1);
        solVec.reserve(nSteps + 1);

        // initial point
        timeVec.push_back(0.0f);
        solVec.push_back(yStart);

        // need one starter step (Euler) to have two points
        float tPrev = timeVec.back();
        float yPrev = solVec.back();
        float fPrev = rhs(tPrev, yPrev);

        float tNext = tPrev + dt;
        float yEuler = yPrev + dt * fPrev;          // Euler predictor
        timeVec.push_back(tNext);
        solVec.push_back(yEuler);
        float fCurr = rhs(tNext, yEuler);

        int curIdx = 2;                             // we already have two points
        while (curIdx <= nSteps) {
            // Adams‑Bashforth predictor (2‑step)
            float pred = solVec[curIdx - 1] + dt * (1.5f * fCurr - 0.5f * fPrev);

            // Adams‑Moulton corrector (trapezoidal)
            float corr = solVec[curIdx - 1] + dt * 0.5f * (rhs(timeVec[curIdx - 1] + dt, pred) + fCurr);

            // store corrected value
            timeVec.push_back(timeVec[curIdx - 1] + dt);
            solVec.push_back(corr);

            // shift history for next iteration
            fPrev = fCurr;
            fCurr = rhs(timeVec.back(), corr);
            ++curIdx;
        }

        // output
        for (int i = 0; i < (int)timeVec.size(); ++i) {
            std::cout << "t = " << timeVec[i] << " , y = " << solVec[i] << '\n';
        }
    }
};

int main() {
    AdamsMoulton9 solver;

    // First cluster: very fine steps over a short interval (dense region)
    float step_small = 0.001f;
    int   steps_small = 5;          // up to t ≈ 0.005
    float y0_small    = 1.0f;        // initial condition y(0)=1
    solver.solve(step_small, steps_small, y0_small);

    std::cout << "---- sparse region ----\n";

    // Second cluster: large steps over a longer interval (sparse region)
    float step_large = 0.5f;
    int   steps_large = 4;          // up to t = 2.0
    float y0_large    = 1.0f;        // same initial condition for independent run
    solver.solve(step_large, steps_large, y0_large);

    return 0;
}
