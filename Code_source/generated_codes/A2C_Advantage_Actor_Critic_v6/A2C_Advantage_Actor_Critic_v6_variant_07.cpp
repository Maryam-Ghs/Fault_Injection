/* LLM input variant 7: reverse-adversarial */
// A2C implementation – Version 6 (modified for reverse‑adversarial input)

#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>

// ------------------------------------------------------------
// Helper: softmax for a pair of logits (2 actions)
// ------------------------------------------------------------
void softmax_pair(const std::vector<float>& logits, std::vector<float>& probs)
{
    float e0 = std::exp(logits[0]);
    float e1 = std::exp(logits[1]);
    float sum = e0 + e1;
    probs[0] = e0 / sum;
    probs[1] = e1 / sum;
}

// ------------------------------------------------------------
// Helper: sample an action from a probability distribution
// ------------------------------------------------------------
int sample_action(const std::vector<float>& probs)
{
    float r = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
    return (r < probs[0]) ? 0 : 1;
}

// ------------------------------------------------------------
// Helper: run a single episode and record (state, action, reward)
// ------------------------------------------------------------
void run_episode(
    const std::vector<std::vector<int> >& nxt_state,
    const std::vector<std::vector<float> >& rew,
    const std::vector<std::vector<float> >& w_policy_a0,
    const std::vector<std::vector<float> >& w_policy_a1,
    std::vector<int>& ep_states,
    std::vector<int>& ep_actions,
    std::vector<float>& ep_rewards,
    int max_steps)
{
    int cur_state = 0;
    int step = 0;
    while (step < max_steps)
    {
        float logit0 = w_policy_a0[0][cur_state];
        float logit1 = w_policy_a1[0][cur_state];
        std::vector<float> logits(2);
        logits[0] = logit0;
        logits[1] = logit1;

        std::vector<float> probs(2);
        softmax_pair(logits, probs);

        int act = sample_action(probs);

        ep_states.push_back(cur_state);
        ep_actions.push_back(act);
        ep_rewards.push_back(rew[cur_state][act]);

        cur_state = nxt_state[cur_state][act];
        ++step;
    }
}

// ------------------------------------------------------------
// Helper: compute discounted returns G_t
// ------------------------------------------------------------
void compute_returns(const std::vector<float>& rewards, float gamma, std::vector<float>& returns)
{
    int T = static_cast<int>(rewards.size());
    returns.resize(T);
    float G = 0.0f;
    for (int i = T - 1; i >= 0; --i)
    {
        G = rewards[i] + gamma * G;
        returns[i] = G;
    }
}

// ------------------------------------------------------------
// Helper: compute state values using current value weights
// ------------------------------------------------------------
void compute_values(const std::vector<int>& states,
                    const std::vector<std::vector<float> >& w_val,
                    std::vector<float>& values)
{
    int N = static_cast<int>(states.size());
    values.resize(N);
    for (int i = 0; i < N; ++i)
    {
        int s = states[i];
        values[i] = w_val[0][s];
    }
}

// ------------------------------------------------------------
// Helper: compute advantages A_t = G_t - V(s_t)
// ------------------------------------------------------------
void compute_advantages(const std::vector<float>& returns,
                        const std::vector<float>& values,
                        std::vector<float>& adv)
{
    int N = static_cast<int>(returns.size());
    adv.resize(N);
    for (int i = 0; i < N; ++i)
    {
        adv[i] = returns[i] - values[i];
    }
}

// ------------------------------------------------------------
// Helper: update policy weights with gradient ascent on advantage
// ------------------------------------------------------------
void update_policy(const std::vector<int>& states,
                   const std::vector<int>& actions,
                   const std::vector<float>& adv,
                   std::vector<std::vector<float> >& w_policy_a0,
                   std::vector<std::vector<float> >& w_policy_a1,
                   float lr)
{
    int N = static_cast<int>(states.size());
    for (int i = 0; i < N; ++i)
    {
        int s = states[i];
        int a = actions[i];
        float logit0 = w_policy_a0[0][s];
        float logit1 = w_policy_a1[0][s];
        std::vector<float> logits(2);
        logits[0] = logit0;
        logits[1] = logit1;
        std::vector<float> probs(2);
        softmax_pair(logits, probs);

        if (a == 0)
        {
            w_policy_a0[0][s] += lr * adv[i] * (1.0f - probs[0]);
            w_policy_a1[0][s] -= lr * adv[i] * probs[1];
        }
        else
        {
            w_policy_a0[0][s] -= lr * adv[i] * probs[0];
            w_policy_a1[0][s] += lr * adv[i] * (1.0f - probs[1]);
        }
    }
}

// ------------------------------------------------------------
// Helper: update value weights with gradient descent on MSE
// ------------------------------------------------------------
void update_value(const std::vector<int>& states,
                  const std::vector<float>& returns,
                  std::vector<std::vector<float> >& w_val,
                  float lr)
{
    int N = static_cast<int>(states.size());
    for (int i = 0; i < N; ++i)
    {
        int s = states[i];
        float V = w_val[0][s];
        float diff = V - returns[i];
        w_val[0][s] -= lr * diff;
    }
}

// ------------------------------------------------------------
// Main driver
// ------------------------------------------------------------
int main()
{
    std::srand(static_cast<unsigned>(std::time(0)));

    // --------------------------------------------------------
    // Reverse‑ordered MDP (5 states, 2 actions)
    // --------------------------------------------------------
    const int NUM_STATES = 5;
    std::vector<std::vector<int> > nxt_state(NUM_STATES, std::vector<int>(2));
    // Reverse progression: 0->4, 1->3, 2->2, 3->1, 4->0 (for both actions)
    nxt_state[0][0] = 4; nxt_state[0][1] = 4;
    nxt_state[1][0] = 3; nxt_state[1][1] = 3;
    nxt_state[2][0] = 2; nxt_state[2][1] = 2;
    nxt_state[3][0] = 1; nxt_state[3][1] = 1;
    nxt_state[4][0] = 0; nxt_state[4][1] = 0;

    // Reward table: high rewards at high‑index states, decreasing backwards
    std::vector<std::vector<float> > rew(NUM_STATES, std::vector<float>(2));
    rew[0][0] = -2.0f; rew[0][1] = -1.5f;
    rew[1][0] = -1.0f; rew[1][1] = -0.5f;
    rew[2][0] = 0.0f;  rew[2][1] = 0.0f;
    rew[3][0] = 1.0f;  rew[3][1] = 1.5f;
    rew[4][0] = 2.0f;  rew[4][1] = 2.5f;

    // --------------------------------------------------------
    // Initialise policy and value parameters (deterministically descending)
    // --------------------------------------------------------
    std::vector<std::vector<float> > w_policy_a0(1, std::vector<float>(NUM_STATES));
    std::vector<std::vector<float> > w_policy_a1(1, std::vector<float>(NUM_STATES));
    std::vector<std::vector<float> > w_val(1, std::vector<float>(NUM_STATES));
    for (int i = 0; i < NUM_STATES; ++i)
    {
        // descending values from 0.04 to -0.04
        float base = 0.04f * (NUM_STATES - 1 - i) / (NUM_STATES - 1);
        w_policy_a0[0][i] = base;
        w_policy_a1[0][i] = -base;
        w_val[0][i]      = base;
    }

    // --------------------------------------------------------
    // Training hyper‑parameters (adversarially chosen)
    // --------------------------------------------------------
    int total_episodes = 15;
    int max_steps_per_ep = 10;
    float gamma = 0.95f;
    float lr_policy = 0.03f;
    float lr_value  = 0.08f;

    // --------------------------------------------------------
    // Training loop
    // --------------------------------------------------------
    for (int ep = 0; ep < total_episodes; ++ep)
    {
        std::vector<int> ep_states;
        std::vector<int> ep_actions;
        std::vector<float> ep_rewards;

        run_episode(nxt_state, rew, w_policy_a0, w_policy_a1,
                    ep_states, ep_actions, ep_rewards,
                    max_steps_per_ep);

        std::vector<float> returns;
        compute_returns(ep_rewards, gamma, returns);

        std::vector<float> values;
        compute_values(ep_states, w_val, values);

        std::vector<float> adv;
        compute_advantages(returns, values, adv);

        update_policy(ep_states, ep_actions, adv,
                       w_policy_a0, w_policy_a1, lr_policy);

        update_value(ep_states, returns, w_val, lr_value);
    }

    // --------------------------------------------------------
    // Print final policy probabilities for each state
    // --------------------------------------------------------
    std::cout << "Final policy (probability of action 0) per state:\n";
    for (int s = 0; s < NUM_STATES; ++s)
    {
        std::vector<float> logits = { w_policy_a0[0][s], w_policy_a1[0][s] };
        std::vector<float> probs(2);
        softmax_pair(logits, probs);
        std::cout << "State " << s << ": " << probs[0] << "\n";
    }

    // --------------------------------------------------------
    // Print final state values
    // --------------------------------------------------------
    std::cout << "\nFinal state values:\n";
    for (int s = 0; s < NUM_STATES; ++s)
    {
        std::cout << "State " << s << ": " << w_val[0][s] << "\n";
    }

    return 0;
}
