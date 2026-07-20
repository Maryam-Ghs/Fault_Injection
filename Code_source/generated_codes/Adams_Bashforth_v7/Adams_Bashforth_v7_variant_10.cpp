/* LLM input variant 10: large-safe-stress */
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
        for (int i = 0; i < 3 && (int)timeline.size() < totalSteps; ++i) {
            float f_val = f(t_cur, y_cur);
            y_cur = y_cur + stepSize * f_val;   // Euler step
            t_cur = t_cur + stepSize;
            timeline.push_back(t_cur);
            values.push_back(y_cur);
        }

        // ---------- Adams‑Bashforth 4‑step ----------
        while ((int)timeline.size() < totalSteps) {
            int idx = (int)timeline.size() - 1; // index of the latest point (y_n)
            // f at n, n-1, n-2, n-3
            float f0 = f(timeline[idx],     values[idx]);
            float f1 = f(timeline[idx - 1], values[idx - 1]);
            float f2 = f(timeline[idx - 2], values[idx - 2]);
            float f3 = f(timeline[idx - 3], values[idx - 3]);

            float y_next = values[idx] + stepSize *
                (55.0f * f0 - 59.0f * f1 + 37.0f * f2 - 9.0f * f3) / 24.0f;
            float t_next = timeline[idx] + stepSize;

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
    // ----------- deterministic large‑scale test vector ------------
    float h = 0.01f;          // smaller step size for stability
    int   n = 1000;          // larger number of points (including t0)

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
