/ * LLM input variant 9: medium-deterministic-random * /
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <algorithm>

// ------------------------------------------------------------
// Simple Advantage Actor‑Critic (A2C) on a deterministic
// medium‑sized environment. Everything lives on the stack, only
// float and int are used and the code is deliberately verbose.
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
    // Fixed maximum sizes for stack‑allocated tables
    // --------------------------------------------------------------------
    static const int MAX_STATES = 6;
    static const int MAX_ACTIONS = 3;
    static const int MAX_STEPS   = 12;

    // --------------------------------------------------------------------
    // Parameter tables (stack allocated)
    // --------------------------------------------------------------------
    float   wPolicy[MAX_STATES][MAX_ACTIONS]; // policy logits for each (state,action)
    float   wValue[MAX_STATES];               // state‑value estimates

    // --------------------------------------------------------------------
    // Constructor – creates a 6‑state, 3‑action problem
    // --------------------------------------------------------------------
    A2C_Solver()
    {
        // Fixed medium problem size
        num_states   = 6;
        num_actions  = 3;
        learning_rate_pi = 0.08f;
        learning_rate_v  = 0.08f;
        gamma        = 0.85f;
        max_episodes = 7;

        // Initialise parameters to deterministic pseudo‑random numbers
        std::srand(0);
        for (int s = 0; s < num_states; ++s) {
            for (int a = 0; a < num_actions; ++a) {
                wPolicy[s][a] = static_cast<float>(std::rand() % 1000) / 5000.0f; // ~0.0‑0.1998
            }
            wValue[s] = static_cast<float>(std::rand() % 1000) / 5000.0f;
        }
    }

    // --------------------------------------------------------------------
    // Softmax – converts logits to probabilities (expanded steps)
    // --------------------------------------------------------------------
    void softmax_row(int state_idx, float probs[MAX_ACTIONS])
    {
        float sum = 0.0f;
        float exp_vals[MAX_ACTIONS];
        for (int a = 0; a < num_actions; ++a) {
            exp_vals[a] = std::exp(wPolicy[state_idx][a]);
            sum += exp_vals[a];
        }
        for (int a = 0; a < num_actions; ++a) {
            probs[a] = exp_vals[a] / sum;
        }
    }

    // --------------------------------------------------------------------
    // Simple deterministic transition table (hard‑coded)
    // --------------------------------------------------------------------
    void step_environment(int cur_state, int act,
                          int& next_state, float& reward, bool& terminal)
    {
        // Transition rules: state = (state + action) % num_states
        next_state = (cur_state + act) % num_states;

        // Reward is +1 if we land on the last state, else 0
        reward = (next_state == num_states - 1) ? 1.0f : 0.0f;

        // Episode ends when we reach the last state
        terminal = (next_state == num_states - 1);
    }

    // --------------------------------------------------------------------
    // Run a single episode, storing the trajectory on the stack
    // --------------------------------------------------------------------
    void run_one_episode(int traj_state[MAX_STEPS],
                         int traj_action[MAX_STEPS],
                         float traj_reward[MAX_STEPS],
                         int& traj_len)
    {
        // Initialise start state
        int cur_state = 0;
        int step_cnt  = 0;
        bool done     = false;

        while (!done && step_cnt < MAX_STEPS) {
            // 1) compute policy probabilities for current state
            float pi[MAX_ACTIONS];
            softmax_row(cur_state, pi);

            // 2) select action with highest probability (deterministic)
            int chosen_action = std::distance(pi, std::max_element(pi, pi + num_actions));

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
    void compute_returns(const float rewards[MAX_STEPS],
                         int length,
                         float returns[MAX_STEPS])
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
    void update_parameters(const int states[MAX_STEPS],
                           const int actions[MAX_STEPS],
                           const float returns[MAX_STEPS],
                           int length)
    {
        // Loop over each step in the trajectory
        for (int t = 0; t < length; ++t) {
            int s = states[t];
            int a = actions[t];
            float G = returns[t];                 // total return
            float V = wValue[s];                  // current value estimate
            float advantage = G - V;               // A(s,a) = G - V(s)

            // ------------------------------------------------------------
            // 1) Update value function: V(s) <- V(s) + lr_v * advantage
            // ------------------------------------------------------------
            float delta_v = learning_rate_v * advantage;
            wValue[s] = wValue[s] + delta_v;      // explicit addition

            // ------------------------------------------------------------
            // 2) Update policy logits via policy gradient:
            //    π(s,a) += lr_pi * advantage * (1 - π(s,a))   for taken action
            //    π(s,b) -= lr_pi * advantage * π(s,b)         for other actions
            // ------------------------------------------------------------
            // a) recompute current policy probabilities (again, verbose)
            float prob[MAX_ACTIONS];
            softmax_row(s, prob);

            // b) compute gradient for each action
            for (int act = 0; act < num_actions; ++act) {
                // indicator = 1 if act == a else 0
                int indicator = (act == a) ? 1 : 0;

                // grad = (indicator - prob[act]) * advantage
                float grad = static_cast<float>(indicator) - prob[act];
                grad *= advantage;

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
        int    traj_state[MAX_STEPS];
        int    traj_action[MAX_STEPS];
        float  traj_reward[MAX_STEPS];
        float  traj_return[MAX_STEPS];
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
                float p[MAX_ACTIONS];
                softmax_row(s, p);
                std::cout << "    π(" << s << ",0) = " << p[0];
                for (int a = 1; a < num_actions; ++a) {
                    std::cout << ", π(" << s << "," << a << ") = " << p[a];
                }
                std::cout << "\n";
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
