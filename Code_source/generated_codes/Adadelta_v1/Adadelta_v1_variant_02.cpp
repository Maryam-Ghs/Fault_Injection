#include <iostream>
#include <cmath>

/* LLM input variant 2: small-diverse */

int main() {
    // problem size and hyper‑parameters
    int sz = 5;                      // length of the parameter vector
    int it = 4;                      // number of Adadelta steps
    float rho = 0.95f;               // decay factor
    float eps = 1e-6f;               // numerical stability term

    // heap‑allocated arrays (no const, no double)
    float *p = new float[sz];        // parameters
    float *g = new float[sz];        // gradients
    float *ag = new float[sz];       // accumulated squared gradients
    float *au = new float[sz];       // accumulated squared updates

    // initialise everything to zero
    for (int i = 0; i < sz; ++i) {
        p[i] = 0.0f;
        ag[i] = 0.0f;
        au[i] = 0.0f;
    }

    // deterministic gradient vectors for each step
    const float preset_grad[4][5] = {
        { 0.1f, -0.2f, 0.05f, -0.15f, 0.0f },
        { -0.05f, 0.25f, -0.1f, 0.2f, -0.05f },
        { 0.3f, -0.1f, 0.0f, 0.05f, -0.2f },
        { -0.2f, 0.15f, -0.05f, -0.1f, 0.25f }
    };

    // main Adadelta loop
    for (int step = 0; step < it; ++step) {
        // load deterministic gradient vector for this step
        for (int i = 0; i < sz; ++i) {
            g[i] = preset_grad[step][i];
        }

        // one Adadelta update for every component
        for (int i = 0; i < sz; ++i) {
            float gg = g[i] * g[i];                           // g²
            ag[i] = rho * ag[i] + (1.0f - rho) * gg;           // Eg² ← ρ·Eg² + (1‑ρ)·g²

            // reordered arithmetic: compute sqrt terms first
            float root_au = std::sqrt(au[i] + eps);           // √(Edx²+ε)
            float root_ag = std::sqrt(ag[i] + eps);           // √(Eg²+ε)

            float upd = - (root_au / root_ag) * g[i];         // Δx
            p[i] += upd;                                      // x ← x + Δx

            float uu = upd * upd;                              // Δx²
            au[i] = rho * au[i] + (1.0f - rho) * uu;           // Edx² ← ρ·Edx² + (1‑ρ)·Δx²
        }

        // print intermediate parameters (one line per step)
        std::cout << "step " << step + 1 << ": ";
        for (int i = 0; i < sz; ++i) {
            std::cout << p[i] << (i + 1 == sz ? '\n' : ' ');
        }
    }

    // clean up heap memory
    delete[] p;
    delete[] g;
    delete[] ag;
    delete[] au;
    return 0;
}
