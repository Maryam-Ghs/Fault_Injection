/* LLM input variant 3: zeros-and-ones */
// A2C implementation – Version 6 (variant emphasizing zeros, ones, identity)

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
    if (r < probs[0])
        return 0;
    else
        return 1;
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
        step = step + 1;
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
    int i = T - 1;
    while (i >= 0)
    {
        G = rewards[i] + gamma * G;
        returns[i] = G;
        i = i - 1;
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
    int i = 0;
    while (i < N)
    {
        int s = states[i];
        values[i] = w_val[0][s];
        i = i + 1;
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
    int i = 0;
    while (i < N)
    {
        adv[i] = returns[i] - values[i];
        i = i + 1;
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
    int i = 0;
    while (i < N)
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
            w_policy_a0[0][s] = w_policy_a0[0][s] + lr * adv[i] * (1.0f - probs[0]);
            w_policy_a1[0][s] = w_policy_a1[0][s] - lr * adv[i] * probs[1];
        }
        else
        {
            w_policy_a0[0][s] = w_policy_a0[0][s] - lr * adv[i] * probs[0];
            w_policy_a1[0][s] = w_policy_a1[0][s] + lr * adv[i] * (1.0f - probs[1]);
        }
        i = i + 1;
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
    int i = 0;
    while (i < N)
    {
        int s = states[i];
        float V = w_val[0][s];
        float target = returns[i];
        float diff = V - target;
        w_val[0][s] = w_val[0][s] - lr * diff;
        i = i + 1;
    }
}

// ------------------------------------------------------------
// Main driver
// ------------------------------------------------------------
int main()
{
    std::srand(static_cast<unsigned>(std::time(0)));

    // --------------------------------------------------------
    // Small predefined MDP (identity transitions)
    // --------------------------------------------------------
    // number of states = 3, actions = 2
    std::vector<std::vector<int> > nxt_state(3, std::vector<int>(2));
    for (int s = 0; s < 3; ++s)
    {
        nxt_state[s][0] = s; // action 0 stays in same state
        nxt_state[s][1] = s; // action 1 also stays in same state
    }

    // reward table: reward[s][a] = 0 for action 0, 1 for action 1
    std::vector<std::vector<float> > rew(3, std::vector<float>(2));
    for (int s = 0; s < 3; ++s)
    {
        rew[s][0] = 0.0f;
        rew[s][1] = 1.0f;
    }

    // --------------------------------------------------------
    // Initialise policy and value parameters (all zeros)
    // --------------------------------------------------------
    std::vector<std::vector<float> > w_policy_a0(1, std::vector<float>(3, 0.0f));
    std::vector<std::vector<float> > w_policy_a1(1, std::vector<float>(3, 0.0f));
    std::vector<std::vector<float> > w_val(1, std::vector<float>(3, 0.0f));

    // --------------------------------------------------------
    // Training hyper‑parameters
    // --------------------------------------------------------
    int total_episodes = 10;
    int max_steps_per_ep = 5;
    float gamma = 1.0f;      // emphasize full return
    float lr_policy = 0.1f;  // larger step to see change
    float lr_value  = 0.1f;

    // --------------------------------------------------------
    // Training loop
    // --------------------------------------------------------
    int ep = 0;
    while (ep < total_episodes)
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

        ep = ep + 1;
    }

    // --------------------------------------------------------
    // Print final policy probabilities for each state
    // --------------------------------------------------------
    std::cout << "Final policy (probability of action 0) per state:\n";
    for (int s = 0; s < 3; ++s)
    {
        float logit0 = w_policy_a0[0][s];
        float logit1 = w_policy_a1[0][s];
        std::vector<float> logits = {logit0, logit1};
        std::vector<float> probs(2);
        softmax_pair(logits, probs);
        std::cout << "State " << s << ": " << probs[0] << "\n";
    }

    // --------------------------------------------------------
    // Print final state values
    // --------------------------------------------------------
    std::cout << "\nFinal state values:\n";
    for (int s = 0; s < 3; ++s)
    {
        std::cout << "State " << s << ": " << w_val[0][s] << "\n";
    }

    return 0;
}
