#include <iostream>
#include <cmath>

/* LLM input variant 9: medium-deterministic-random */

float deterministic_gradient(int step, int idx) {
    // Simple deterministic pseudo‑random generator based on step and index
    unsigned int val = static_cast<unsigned int>((step + 1) * 31u + (idx + 1) * 17u);
    // Modulo to keep within a reasonable range, then scale to [-0.5, 0.5]
    return (static_cast<float>(val % 10000u) / 10000.0f) - 0.5f;
}

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
    float *au = new float[sz];        // accumulated squared updates

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
        // generate a deterministic gradient vector in [-0.5,0.5]
        idx = 0;
        while (idx < sz) {
            g[idx] = deterministic_gradient(step, idx);
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
