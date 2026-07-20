/* LLM input variant 3: zeros-and-ones */
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>

// ------------------------------------------------------------
// Simple Advantage Actor‑Critic (A2C) on a tiny deterministic
// environment. Everything lives on the stack, only float and int
// are used and the code is deliberately verbose.
// ------------------------------------------------------------

class A2C_Solver {
public:
    // --------------------------------------------------------------------
    // Hyper‑parameters (all float or int)
    // --------------------------------------------------------------------
    int     num_states;          // number of discrete states
    int     num_actions;         // number of discrete actions
    float   learning_rate_pi;    // policy step size
    float   learning_rate_v;    // value step size
    float   gamma;               // discount factor
    int     max_episodes;        // how many episodes to run

    // --------------------------------------------------------------------
    // Parameter tables (stack allocated)
    // --------------------------------------------------------------------
    float   wPolicy[4][2];       // policy logits for each (state,action)
    float   wValue[4];           // state‑value estimates

    // --------------------------------------------------------------------
    // Constructor – creates a tiny 4‑state, 2‑action problem
    // --------------------------------------------------------------------
    A2C_Solver()
    {
        // Fixed small problem size
        num_states   = 4;
        num_actions  = 2;
        learning_rate_pi = 1.0f;   // identity‑like step size
        learning_rate_v  = 1.0f;   // identity‑like step size
        gamma        = 1.0f;       // no discounting
        max_episodes = 5;

        // Initialise parameters to zeros (policy) and ones (value)
        for (int s = 0; s < num_states; ++s) {
            for (int a = 0; a < num_actions; ++a) {
                wPolicy[s][a] = 0.0f;            // zero logits → 0.5 prob each
            }
            wValue[s] = 1.0f;                    // identity‑like value
        }
    }

    // --------------------------------------------------------------------
    // Softmax – converts logits to probabilities (expanded steps)
    // --------------------------------------------------------------------
    void softmax_row(int state_idx, float probs[2])
    {
        // 1) exponentiate each logit
        float e0 = std::exp(wPolicy[state_idx][0]);
        float e1 = std::exp(wPolicy[state_idx][1]);

        // 2) compute denominator
        float denom = e0 + e1;

        // 3) normalise
        probs[0] = e0 / denom;
        probs[1] = e1 / denom;
    }

    // --------------------------------------------------------------------
    // Simple deterministic transition table (hard‑coded)
    // --------------------------------------------------------------------
    void step_environment(int cur_state, int act,
                          int& next_state, float& reward, bool& terminal)
    {
        // Transition rules: state = (state + action) % 4
        next_state = (cur_state + act) % num_states;

        // Reward is +1 if we land on state 0 (zero), else 0
        reward = (next_state == 0) ? 1.0f : 0.0f;

        // Episode ends when we reach state 0
        terminal = (next_state == 0);
    }

    // --------------------------------------------------------------------
    // Run a single episode, storing the trajectory on the stack
    // --------------------------------------------------------------------
    void run_one_episode(int traj_state[10],
                         int traj_action[10],
                         float traj_reward[10],
                         int& traj_len)
    {
        // Initialise start state
        int cur_state = 0;
        int step_cnt  = 0;
        bool done     = false;

        while (!done && step_cnt < 10) {
            // 1) compute policy probabilities for current state
            float pi[2];
            softmax_row(cur_state, pi);

            // 2) sample action (here we pick the larger prob for determinism)
            int chosen_action = (pi[0] >= pi[1]) ? 0 : 1;

            // 3) record trajectory entry
            traj_state[step_cnt]  = cur_state;
            traj_action[step_cnt] = chosen_action;

            // 4) step the environment
            int nxt_state;
            float rew;
            bool term;
            step_environment(cur_state, chosen_action,
                             nxt_state, rew, term);

            // 5) store obtained reward
            traj_reward[step_cnt] = rew;

            // 6) prepare for next iteration
            cur_state = nxt_state;
            done      = term;
            ++step_cnt;
        }
        traj_len = step_cnt;
    }

    // --------------------------------------------------------------------
    // Compute discounted returns (expanded loops)
    // --------------------------------------------------------------------
    void compute_returns(const float rewards[10],
                         int length,
                         float returns[10])
    {
        // Work backwards from the end of the episode
        int idx = length - 1;
        float running_ret = 0.0f;
        while (idx >= 0) {
            // G_t = r_t + gamma * G_{t+1}
            running_ret = rewards[idx] + gamma * running_ret;
            returns[idx] = running_ret;
            --idx;
        }
    }

    // --------------------------------------------------------------------
    // Perform one policy/value update using the collected trajectory
    // --------------------------------------------------------------------
    void update_parameters(const int states[10],
                           const int actions[10],
                           const float returns[10],
                           int length)
    {
        // Loop over each step in the trajectory
        for (int t = 0; t < length; ++t) {
            int s = states[t];
            int a = actions[t];
            float G = returns[t];                 // total return
            float V = wValue[s];                  // current value estimate
            float advantage = G - V;              // A(s,a) = G - V(s)

            // ------------------------------------------------------------
            // 1) Update value function: V(s) <- V(s) + lr_v * advantage
            // ------------------------------------------------------------
            float delta_v = learning_rate_v * advantage;
            wValue[s] = wValue[s] + delta_v;      // explicit addition

            // ------------------------------------------------------------
            // 2) Update policy logits via policy gradient:
            //    π(s,a) += lr_pi * advantage * (1 - π(s,a))   for taken action
            //    π(s,b) -= lr_pi * advantage * π(s,b)         for other action
            // ------------------------------------------------------------
            // a) recompute current policy probabilities (again, verbose)
            float prob[2];
            softmax_row(s, prob);

            // b) compute gradient for each action
            for (int act = 0; act < num_actions; ++act) {
                // indicator = 1 if act == a else 0
                int indicator = (act == a) ? 1 : 0;

                // grad = (indicator - prob[act]) * advantage
                float grad = ((float)indicator - prob[act]) * advantage;

                // step = lr_pi * grad
                float step = learning_rate_pi * grad;

                // apply to logits
                wPolicy[s][act] = wPolicy[s][act] + step;
            }
        }
    }

    // --------------------------------------------------------------------
    // Main training loop – prints policy and value after each episode
    // --------------------------------------------------------------------
    void train()
    {
        // Fixed‑size buffers for a single episode (stack allocated)
        int    traj_state[10];
        int    traj_action[10];
        float  traj_reward[10];
        float  traj_return[10];
        int    traj_len = 0;

        for (int ep = 0; ep < max_episodes; ++ep) {
            // 1) generate an episode
            run_one_episode(traj_state, traj_action, traj_reward, traj_len);

            // 2) compute discounted returns
            compute_returns(traj_reward, traj_len, traj_return);

            // 3) update policy and value tables
            update_parameters(traj_state, traj_action, traj_return, traj_len);

            // 4) print current parameters (verbose)
            std::cout << "Episode " << ep + 1 << " finished, length = "
                      << traj_len << "\n";

            std::cout << "  State‑value estimates:\n";
            for (int s = 0; s < num_states; ++s) {
                std::cout << "    V[" << s << "] = " << wValue[s] << "\n";
            }

            std::cout << "  Policy (softmax probabilities):\n";
            for (int s = 0; s < num_states; ++s) {
                float p[2];
                softmax_row(s, p);
                std::cout << "    π(" << s << ",0) = " << p[0]
                          << ", π(" << s << ",1) = " << p[1] << "\n";
            }
            std::cout << "-------------------------------------------------\n";
        }
    }
};

// --------------------------------------------------------------------
// Entry point – generates the problem internally and prints results.
// --------------------------------------------------------------------
int main()
{
    // Seed randomness for reproducibility (optional)
    std::srand(42);

    // Instantiate solver and run training
    A2C_Solver solver;
    solver.train();

    return 0;
}
