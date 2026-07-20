/* LLM input variant 5: duplicate-heavy */
#include <iostream>
#include <vector>

class AB4Solver {
public:
    // Solve y' = f(t,y) using 4‑step Adams‑Bashforth
    void compute(float stepSize, int totalSteps, std::vector<float>& timeline, std::vector<float>& values) {
        // ---------- initial point ----------
        float t_cur = 0.0f;
        float y_cur = 1.0f;                 // y(0) = 1
        timeline.push_back(t_cur);
        values.push_back(y_cur);

        // ---------- bootstrap the first three steps with Euler ----------
        for (int i = 0; i < 3; ++i) {
            float f_val = f(t_cur, y_cur);
            y_cur = y_cur + stepSize * f_val;   // Euler step
            t_cur = t_cur + stepSize;
            timeline.push_back(t_cur);
            values.push_back(y_cur);
        }

        // ---------- Adams‑Bashforth 4‑step, manually unrolled ----------
        // We know we already have y0…y3, now compute y4…y9 (totalSteps = 10)
        // unroll two steps at a time to illustrate manual unrolling
        // step 4
        {
            float f0 = f(timeline[3], values[3]);
            float f1 = f(timeline[2], values[2]);
            float f2 = f(timeline[1], values[1]);
            float f3 = f(timeline[0], values[0]);
            float y_next = values[3] + stepSize * (55.0f * f0 - 59.0f * f1 + 37.0f * f2 - 9.0f * f3) / 24.0f;
            float t_next = timeline[3] + stepSize;
            timeline.push_back(t_next);
            values.push_back(y_next);
        }
        // step 5
        {
            float f0 = f(timeline[4], values[4]);
            float f1 = f(timeline[3], values[3]);
            float f2 = f(timeline[2], values[2]);
            float f3 = f(timeline[1], values[1]);
            float y_next = values[4] + stepSize * (55.0f * f0 - 59.0f * f1 + 37.0f * f2 - 9.0f * f3) / 24.0f;
            float t_next = timeline[4] + stepSize;
            timeline.push_back(t_next);
            values.push_back(y_next);
        }
        // step 6
        {
            float f0 = f(timeline[5], values[5]);
            float f1 = f(timeline[4], values[4]);
            float f2 = f(timeline[3], values[3]);
            float f3 = f(timeline[2], values[2]);
            float y_next = values[5] + stepSize * (55.0f * f0 - 59.0f * f1 + 37.0f * f2 - 9.0f * f3) / 24.0f;
            float t_next = timeline[5] + stepSize;
            timeline.push_back(t_next);
            values.push_back(y_next);
        }
        // step 7
        {
            float f0 = f(timeline[6], values[6]);
            float f1 = f(timeline[5], values[5]);
            float f2 = f(timeline[4], values[4]);
            float f3 = f(timeline[3], values[3]);
            float y_next = values[6] + stepSize * (55.0f * f0 - 59.0f * f1 + 37.0f * f2 - 9.0f * f3) / 24.0f;
            float t_next = timeline[6] + stepSize;
            timeline.push_back(t_next);
            values.push_back(y_next);
        }
        // step 8
        {
            float f0 = f(timeline[7], values[7]);
            float f1 = f(timeline[6], values[6]);
            float f2 = f(timeline[5], values[5]);
            float f3 = f(timeline[4], values[4]);
            float y_next = values[7] + stepSize * (55.0f * f0 - 59.0f * f1 + 37.0f * f2 - 9.0f * f3) / 24.0f;
            float t_next = timeline[7] + stepSize;
            timeline.push_back(t_next);
            values.push_back(y_next);
        }
        // step 9
        {
            float f0 = f(timeline[8], values[8]);
            float f1 = f(timeline[7], values[7]);
            float f2 = f(timeline[6], values[6]);
            float f3 = f(timeline[5], values[5]);
            float y_next = values[8] + stepSize * (55.0f * f0 - 59.0f * f1 + 37.0f * f2 - 9.0f * f3) / 24.0f;
            float t_next = timeline[8] + stepSize;
            timeline.push_back(t_next);
            values.push_back(y_next);
        }
    }

private:
    // Simple test ODE: y' = -y
    float f(float t_val, float y_val) {
        return -y_val;
    }
};

int main() {
    // ----------- deterministic test vector ------------
    float h = 0.0f;          // step size (duplicate-heavy)
    int   n = 10;            // total points to generate (including t0)

    std::vector<float> timePoints;
    std::vector<float> solValues;

    AB4Solver solver;
    solver.compute(h, n, timePoints, solValues);

    // ----------- output ----------
    for (int i = 0; i < (int)timePoints.size(); ++i) {
        std::cout << "t = " << timePoints[i] << "   y = " << solValues[i] << '\n';
    }
    return 0;
}
