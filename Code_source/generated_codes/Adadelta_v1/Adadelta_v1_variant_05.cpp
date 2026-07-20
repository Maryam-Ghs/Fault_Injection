#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>

/* LLM input variant 5: duplicate-heavy */

int main() {
    // problem size and hyper‑parameters
    int sz = 12;                     // length of the parameter vector
    int it = 7;                      // number of Adadelta steps
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

    // main Adadelta loop
    int step = 0;
    while (step < it) {
        // deterministic duplicate‑heavy gradient vector
        idx = 0;
        while (idx < sz) {
            // pattern with many repeated values: 0.2, 0.2, -0.2, -0.2, ...
            g[idx] = (idx % 4 < 2) ? 0.2f : -0.2f;
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
