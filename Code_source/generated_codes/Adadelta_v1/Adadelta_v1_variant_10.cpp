/* LLM input variant 10: large-safe-stress */
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>

int main() {
    // problem size and hyper‑parameters
    int sz = 1000;                  // length of the parameter vector (larger stress size)
    int it = 50;                    // number of Adadelta steps (larger stress count)
    float rho = 0.95f;              // decay factor
    float eps = 1e-6f;              // numerical stability term

    // heap‑allocated arrays (no const, no double)
    float *p = new float[sz];       // parameters
    float *g = new float[sz];       // gradients
    float *ag = new float[sz];      // accumulated squared gradients
    float *au = new float[sz];      // accumulated squared updates

    // initialise everything to zero
    int idx = 0;
    while (idx < sz) {
        p[idx] = 0.0f;
        ag[idx] = 0.0f;
        au[idx] = 0.0f;
        ++idx;
    }

    // seed random generator with a fixed seed for deterministic behavior
    std::srand(42U);

    // main Adadelta loop
    int step = 0;
    while (step < it) {
        // generate a new random gradient vector (uniform in [-0.5,0.5])
        idx = 0;
        while (idx < sz) {
            g[idx] = (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) - 0.5f;
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
