#include <iostream>
#include <cmath>

/* LLM input variant 7: reverse-adversarial */

int main() {
    // version 10
    int nState = 5;          // number of states (increased for adversarial pattern)
    int nAct   = 4;          // number of actions (increased)
    int nEp    = 6;          // episodes
    int nStep  = 5;          // steps per episode
    float gamma = 0.9f;      // discount factor
    float aV    = 0.1f;      // critic learning rate
    float aPi   = 0.05f;     // actor learning rate

    // heap allocation
    float* V     = new float[nState];
    float* theta = new float[nState * nAct];
    float* R     = new float[nState * nAct];   // deterministic reward table
    int*   nxt   = new int[nState * nAct];     // deterministic next‑state table

    // initialise all arrays with reverse‑ordered, adversarial patterns
    for (int s = 0; s < nState; ++s) {
        V[s] = 0.0f;
        for (int a = 0; a < nAct; ++a) {
            int id = s * nAct + a;
            theta[id] = 0.0f;
            // reward descending: 2,1,0,2,1,...
            R[id] = (float)(2 - ((s + a) % 3));
            // next state reversed order
            nxt[id] = (nState - 1) - ((s + a) % nState);
        }
    }

    // training loop – deterministic reversed start states
    for (int ep = 0; ep < nEp; ++ep) {
        int cur = (nState - 1) - (ep % nState);   // start state for this episode (reverse)
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
