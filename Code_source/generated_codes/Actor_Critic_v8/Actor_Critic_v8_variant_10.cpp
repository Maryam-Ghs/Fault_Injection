#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>

/* LLM input variant 10: large-safe-stress */

int main() {
    // ------------------- version #8 -------------------
    // Larger predefined MDP for stress testing (still safe)
    const int numStates = 100;   // increased state space
    const int numActs   = 10;    // increased action set
    const float gamma   = 0.9f;
    const float lrVal   = 0.1f;   // learning rate for value
    const float lrPol   = 0.05f;  // learning rate for policy

    // Seed RNG for reproducibility of generated tables
    std::srand(0);

    // Transition table: nextState[ state ][ action ]
    std::vector< std::vector<int> > nxt(numStates, std::vector<int>(numActs));
    for (int s = 0; s < numStates; ++s) {
        for (int a = 0; a < numActs; ++a) {
            // deterministic wrap‑around transition
            nxt[s][a] = (s + a + 1) % numStates;
        }
    }

    // Reward table: rew[ state ][ action ]
    std::vector< std::vector<float> > rew(numStates, std::vector<float>(numActs));
    for (int s = 0; s < numStates; ++s) {
        for (int a = 0; a < numActs; ++a) {
            // reward cycles between 0 and 4
            rew[s][a] = static_cast<float>((s * a) % 5);
        }
    }

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
    const int episodes = 1000;   // more episodes for stress
    const int maxSteps = 20;     // longer episodes
    for (int epi = 0; epi < episodes; ++epi) {
        int curState = 0;               // start state
        int stepCnt  = 0;
        while (stepCnt < maxSteps) {
            // ---- compute policy probs ----
            std::vector<float> pi(numActs, 0.0f);
            softmax(curState, pi);

            // ---- sample action (simple roulette) ----
            float rnd = static_cast<float>(std::rand()) / RAND_MAX;
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
