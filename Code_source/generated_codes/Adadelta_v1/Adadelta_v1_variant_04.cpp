#include <iostream>
#include <cmath>
/* LLM input variant 4: signed-extremes */
int main() {
    // problem size and hyper‑parameters
    int sz = 10;                     // length of the parameter vector
    int it = 5;                      // number of Adadelta steps
    float rho = 0.95f;               // decay factor
    float eps = 1e-6f;               // numerical stability term

    // heap‑allocated arrays (no const, no double)
    float *p = new float[sz];        // parameters
    float *g = new float[sz];        // gradients
    float *ag = new float[sz];       // accumulated squared gradients
    float *au = new float[sz];       // accumulated squared updates

    // initialise everything to zero
    int idx = 0;
    while (idx < sz) {
        p[idx] = 0.0f;
        ag[idx] = 0.0f;
        au[idx] = 0.0f;
        ++idx;
    }

    // deterministic gradient pattern mixing negative, zero, and positive values
    const float grad_pattern[5] = { -0.5f, -0.25f, 0.0f, 0.25f, 0.5f };

    // main Adadelta loop
    int step = 0;
    while (step < it) {
        // generate a deterministic gradient vector based on the pattern
        idx = 0;
        while (idx < sz) {
            g[idx] = grad_pattern[idx % 5];
            ++idx;
        }

        // one Adadelta update for every component
        idx = 0;
        while (idx < sz) {
            float gg = g[idx] * g[idx];                           // g²
            ag[idx] = rho * ag[idx] + (1.0f - rho) * gg;           // Eg² ← ρ·Eg² + (1‑ρ)·g²

            // reordered arithmetic: compute sqrt terms first
            float root_au = std::sqrt(au[idx] + eps);              // √(Edx²+ε)
            float root_ag = std::sqrt(ag[idx] + eps);              // √(Eg²+ε)

            float upd = - (root_au / root_ag) * g[idx];           // Δx
            p[idx] += upd;                                         // x ← x + Δx

            float uu = upd * upd;                                 // Δx²
            au[idx] = rho * au[idx] + (1.0f - rho) * uu;           // Edx² ← ρ·Edx² + (1‑ρ)·Δx²
            ++idx;
        }

        // print intermediate parameters (one line per step)
        std::cout << "step " << step + 1 << ": ";
        idx = 0;
        while (idx < sz) {
            std::cout << p[idx] << (idx + 1 == sz ? '\n' : ' ');
            ++idx;
        }
        ++step;
    }

    // clean up heap memory
    delete[] p;
    delete[] g;
    delete[] ag;
    delete[] au;
    return 0;
}
