// LLM input variant 9: medium-deterministic-random
// Adams‑Moulton ODE solver – version #7
// ------------------------------------------------------------
// Implements the implicit 2‑step Adams‑Moulton method (trapezoidal)
// using only float and int, std::vector and a fully iterative style.

#include <iostream>
#include <vector>

// right‑hand side of the ODE: y' = -y
float rhs(float t, float y) {
    return -y;
}

// one explicit Euler step – used as a predictor
void eulerPredict(float t_cur, float y_cur, float h, float& t_out, float& y_out) {
    t_out = t_cur + h;
    y_out = y_cur + h * rhs(t_cur, y_cur);
}

// one Adams‑Moulton correction step (trapezoidal rule)
void adamsMoultonCorrect(float t_cur, float y_cur,
                         float t_nxt, float y_pred,
                         float h, float& y_corr) {
    // reorder arithmetic: compute half‑step first, then the sum
    float half_h = h * 0.5f;
    float sum_f = rhs(t_cur, y_cur) + rhs(t_nxt, y_pred);
    y_corr = y_cur + half_h * sum_f;
}

// driver that builds the solution vector
void solveAdamsMoulton(float t_start, float y_start,
                       float h, int steps,
                       std::vector<float>& t_vec,
                       std::vector<float>& y_vec) {
    // initialise containers
    t_vec.clear();
    y_vec.clear();
    t_vec.push_back(t_start);
    y_vec.push_back(y_start);

    // first step – simple Euler to obtain a second point
    float t_tmp, y_tmp;
    eulerPredict(t_start, y_start, h, t_tmp, y_tmp);
    t_vec.push_back(t_tmp);
    y_vec.push_back(y_tmp);

    // iterative loop for the remaining points
    int idx = 1;                     // index of the last known point
    while (idx < steps) {
        float t_cur = t_vec[idx];
        float y_cur = y_vec[idx];

        // predictor (Euler) for the next point
        float t_pred, y_pred;
        eulerPredict(t_cur, y_cur, h, t_pred, y_pred);

        // corrector (Adams‑Moulton)
        float y_corr;
        adamsMoultonCorrect(t_cur, y_cur, t_pred, y_pred, h, y_corr);

        // store the corrected values
        t_vec.push_back(t_pred);     // t_pred == t_cur + h
        y_vec.push_back(y_corr);

        ++idx;
    }
}

// ----------------------------------------------------------------
// main – generates deterministic test data and prints the result
int main() {
    // problem definition (medium‑sized deterministic pseudo‑random‑looking input)
    float t0 = 0.0f;          // start time
    float y0 = 0.1234567f;   // initial value with non‑trivial fraction
    float step = 0.067f;    // unconventional step size
    int   nSteps = 31;       // moderate number of steps (t ≈ 2.077)

    // containers for the solution
    std::vector<float> timeVals;
    std::vector<float> solVals;

    // solve the ODE
    solveAdamsMoulton(t0, y0, step, nSteps, timeVals, solVals);

    // output the table (t, y_approx)
    int i = 0;
    while (i < static_cast<int>(timeVals.size())) {
        std::cout << "t = " << timeVals[i]
                  << " , y ≈ " << solVals[i] << '\n';
        ++i;
    }

    return 0;
}
