#include <iostream>
#include <cmath>

/* LLM input variant 8: sparse-skewed */

int main() {
    // version 10
    int nState = 12;         // number of states (expanded)
    int nAct   = 6;          // number of actions (expanded)
    int nEp    = 8;          // episodes
    int nStep  = 5;          // steps per episode
    float gamma = 0.9f;      // discount factor
    float aV    = 0.1f;      // critic learning rate
    float aPi   = 0.05f;     // actor learning rate

    // heap allocation
    float* V     = new float[nState];
    float* theta = new float[nState * nAct];
    float* R     = new float[nState * nAct];   // deterministic reward table (sparse)
    int*   nxt   = new int[nState * nAct];     // deterministic next‑state table (skewed)

    // initialise all arrays
    for (int s = 0; s < nState; ++s) {
        V[s] = 0.0f;
        for (int a = 0; a < nAct; ++a) {
            int id = s * nAct + a;
            theta[id] = 0.0f;
            // sparse reward pattern: only a few non‑zero entries
            if (s == 0 && a == 0) {
                R[id] = 5.0f;
            } else if (s == 5 && a == 2) {
                R[id] = 2.0f;
            } else if (s == 11 && a == 5) {
                R[id] = -3.0f;
            } else {
                R[id] = 0.0f;
            }
            // skewed next‑state pattern: majority point to state 0
            nxt[id] = 0;
            if (s == 0 && a == 0) {
                nxt[id] = 1;
            } else if (s == 5 && a == 2) {
                nxt[id] = 6;
            } else if (s == 11 && a == 5) {
                nxt[id] = 11; // self‑loop for terminal-like state
            }
        }
    }

    // training loop – deterministic start states
    for (int ep = 0; ep < nEp; ++ep) {
        int cur = ep % nState;        // start state for this episode
        for (int st = 0; st < nStep; ++st) {
            // ---- compute soft‑max denominator (fused with later use) ----
            float den = 0.0f;
            for (int a = 0; a < nAct; ++a) den += expf(theta[cur * nAct + a]);

            // ---- select greedy action (deterministic for test) ----
            int act = 0;
            float best = -1e30f;
            for (int a = 0; a < nAct; ++a) {
                float prob = expf(theta[cur * nAct + a]) / den;
                if (prob > best) { best = prob; act = a; }
            }

            int id   = cur * nAct + act;
            int nxtS = nxt[id];
            float rew = R[id];

            // ---- TD error (critic) ----
            float delta = rew + gamma * V[nxtS] - V[cur];
            // ---- critic update (fused) ----
            V[cur] += aV * delta;

            // ---- actor update (policy‑gradient, fused) ----
            for (int a = 0; a < nAct; ++a) {
                float prob = expf(theta[cur * nAct + a]) / den;
                float grad = (a == act ? 1.0f - prob : -prob);
                theta[cur * nAct + a] += aPi * delta * grad;
            }

            cur = nxtS;   // move to next state
        }
    }

    // ----- output results -----
    std::cout << "Final state values V:\n";
    for (int s = 0; s < nState; ++s) std::cout << V[s] << ' ';
    std::cout << "\n\nFinal policy parameters theta (state‑action):\n";
    for (int s = 0; s < nState; ++s) {
        for (int a = 0; a < nAct; ++a) std::cout << theta[s * nAct + a] << ' ';
        std::cout << '\n';
    }

    // clean up heap memory
    delete[] V;
    delete[] theta;
    delete[] R;
    delete[] nxt;

    return 0;
}
