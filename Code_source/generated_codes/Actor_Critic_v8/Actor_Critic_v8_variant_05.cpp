#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>

/* LLM input variant 5: duplicate-heavy */

int main() {
    // ------------------- version #8 -------------------
    // Small predefined MDP with duplicate-heavy patterns
    int numStates = 4;
    int numActs   = 3;
    float gamma   = 0.9f;
    float lrVal   = 0.1f;   // learning rate for value
    float lrPol   = 0.05f;  // learning rate for policy

    // Transition table: nextState[ state ][ action ] (many duplicates)
    std::vector< std::vector<int> > nxt = {
        {1, 1, 1},
        {2, 2, 2},
        {3, 3, 3},
        {0, 0, 0}
    };

    // Reward table: rew[ state ][ action ] (identical rewards)
    std::vector< std::vector<float> > rew = {
        {0.5f, 0.5f, 0.5f},
        {0.5f, 0.5f, 0.5f},
        {0.5f, 0.5f, 0.5f},
        {0.5f, 0.5f, 0.5f}
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
    int episodes   = 200;
    int maxSteps   = 5;
    for (int epi = 0; epi < episodes; ++epi) {
        int curState = 0;               // start state
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
