#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>

/* LLM input variant 7: reverse-adversarial */
int main() {
    // ------------------- version #8 (variant 7) -------------------
    // Reverse‑ordered, adversarial MDP
    int numStates = 5;          // increased size for more complex traversal
    int numActs   = 2;
    float gamma   = 0.99f;      // near‑max discount to emphasize long‑term effects
    float lrVal   = 0.2f;       // larger learning rate for value (still safe)
    float lrPol   = 0.1f;       // larger learning rate for policy

    // Transition table: nextState[ state ][ action ]
    // Reverse‑ordered deterministic transitions forming a long cycle
    std::vector< std::vector<int> > nxt = {
        {4, 3},   // state 0 -> actions send to high indices
        {0, 4},   // state 1
        {1, 0},   // state 2
        {2, 1},   // state 3
        {3, 2}    // state 4
    };

    // Reward table: rew[ state ][ action ]
    // Adversarial rewards: mostly negative, with a single positive outlier
    std::vector< std::vector<float> > rew = {
        {-5.0f, -4.0f},
        {-3.0f, -2.0f},
        {-1.0f,  0.0f},
        { 0.5f, -0.5f},
        { 2.0f, -1.0f}
    };

    // Value estimates V(s)
    std::vector<float> V(numStates, 0.0f);
    // Policy parameters θ(s,a)
    std::vector< std::vector<float> > theta(numStates, std::vector<float>(numActs, 0.0f));

    // Helper lambda: softmax probabilities for a given state
    auto softmax = [&](int s, std::vector<float>& prob) {
        float maxZ = theta[s][0];
        for (int a = 1; a < numActs; ++a) {
            if (theta[s][a] > maxZ) maxZ = theta[s][a];
        }
        float sumExp = 0.0f;
        for (int a = 0; a < numActs; ++a) {
            float e = expf(theta[s][a] - maxZ); // improve stability
            prob[a] = e;
            sumExp += e;
        }
        for (int a = 0; a < numActs; ++a) {
            prob[a] /= sumExp;
        }
    };

    // Training loop (loop‑heavy, iterative)
    int episodes   = 300;   // more episodes to expose worst‑case learning dynamics
    int maxSteps   = 10;    // longer episode length to stress the cycle
    for (int epi = 0; epi < episodes; ++epi) {
        int curState = numStates - 1;   // start from the highest-index state (adversarial)
        int stepCnt  = 0;
        while (stepCnt < maxSteps) {
            // ---- compute policy probs ----
            std::vector<float> pi(numActs, 0.0f);
            softmax(curState, pi);

            // ---- sample action (simple roulette) ----
            float rnd = static_cast<float>(rand()) / RAND_MAX;
            float accum = 0.0f;
            int act = 0;
            for (int a = 0; a < numActs; ++a) {
                accum += pi[a];
                if (rnd <= accum) { act = a; break; }
            }

            // ---- observe reward and next state ----
            float rewardNow = rew[curState][act];
            int nxtState    = nxt[curState][act];

            // ---- TD‑error (expanded steps) ----
            float vCur   = V[curState];
            float vNxt   = V[nxtState];
            float target = rewardNow + gamma * vNxt;
            float delta  = target - vCur;

            // ---- update value function ----
            V[curState] = vCur + lrVal * delta;

            // ---- update policy parameters ----
            for (int a = 0; a < numActs; ++a) {
                float grad = (a == act) ? (1.0f - pi[a]) : (-pi[a]);
                theta[curState][a] = theta[curState][a] + lrPol * delta * grad;
            }

            // ---- move to next step ----
            curState = nxtState;
            ++stepCnt;
        }
    }

    // ----- Print final value estimates -----
    std::cout << "Final state values (V):\n";
    for (int s = 0; s < numStates; ++s) {
        std::cout << "  V[" << s << "] = " << V[s] << "\n";
    }

    // ----- Print final policy (softmax probs) -----
    std::cout << "\nFinal policy (π) probabilities:\n";
    for (int s = 0; s < numStates; ++s) {
        std::vector<float> prob(numActs, 0.0f);
        softmax(s, prob);
        std::cout << "  State " << s << ": ";
        for (int a = 0; a < numActs; ++a) {
            std::cout << "a" << a << "=" << prob[a] << " ";
        }
        std::cout << "\n";
    }

    return 0;
}
