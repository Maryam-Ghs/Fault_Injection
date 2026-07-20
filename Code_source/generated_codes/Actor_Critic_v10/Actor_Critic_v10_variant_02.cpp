#include <iostream>
#include <cmath>

/* LLM input variant 2: small-diverse */

int main() {
    // version 10 – variant 2
    int nState = 4;          // number of states
    int nAct   = 3;          // number of actions
    int nEp    = 4;          // episodes
    int nStep  = 3;          // steps per episode
    float gamma = 0.85f;      // discount factor
    float aV    = 0.12f;      // critic learning rate
    float aPi   = 0.07f;      // actor learning rate

    // heap allocation
    float* V     = new float[nState];
    float* theta = new float[nState * nAct];
    float* R     = new float[nState * nAct];   // deterministic reward table
    int*   nxt   = new int[nState * nAct];     // deterministic next‑state table

    // initialise all arrays
    for (int s = 0; s < nState; ++s) {
        V[s] = 0.0f;
        for (int a = 0; a < nAct; ++a) {
            int id = s * nAct + a;
            theta[id] = 0.0f;
            // reward varies between -2 and 2
            R[id]     = static_cast<float>(((s * 2 + a * 3) % 5) - 2);
            // next state cycles with a mixed pattern
            nxt[id]   = (s * a + 1) % nState;
        }
    }

    // training loop – deterministic start states
    for (int ep = 0; ep < nEp; ++ep) {
        int cur = (ep * 2) % nState;        // start state for this episode
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
