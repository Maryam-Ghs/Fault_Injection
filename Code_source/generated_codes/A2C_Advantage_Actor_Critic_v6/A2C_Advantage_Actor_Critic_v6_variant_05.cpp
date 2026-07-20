/* LLM input variant 5: duplicate-heavy */
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
    // exponentiate
    float e0 = std::exp(logits[0]);
    float e1 = std::exp(logits[1]);
    float sum = e0 + e1;
    // normalize
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
    // start from state 0
    int cur_state = 0;
    int step = 0;
    while (step < max_steps)
    {
        // ---- compute policy logits for current state ----
        float logit0 = w_policy_a0[0][cur_state]; // weight for action 0
        float logit1 = w_policy_a1[0][cur_state]; // weight for action 1
        std::vector<float> logits(2);
        logits[0] = logit0;
        logits[1] = logit1;

        // ---- compute action probabilities ----
        std::vector<float> probs(2);
        softmax_pair(logits, probs);

        // ---- sample an action ----
        int act = sample_action(probs);

        // ---- store transition ----
        ep_states.push_back(cur_state);
        ep_actions.push_back(act);
        ep_rewards.push_back(rew[cur_state][act]);

        // ---- move to next state ----
        cur_state = nxt_state[cur_state][act];

        // ---- optional termination (small MDP, we stop after max_steps) ----
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
    // go backwards
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
        // linear value: dot product of w_val row and one‑hot state
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
        // recompute logits and probabilities for current state
        float logit0 = w_policy_a0[0][s];
        float logit1 = w_policy_a1[0][s];
        std::vector<float> logits(2);
        logits[0] = logit0;
        logits[1] = logit1;
        std::vector<float> probs(2);
        softmax_pair(logits, probs);

        // gradient for action a: (1 - prob) * adv, for other action: -prob * adv
        if (a == 0)
        {
            w_policy_a0[0][s] = w_policy_a0[0][s] + lr * adv[i] * (1.0f - probs[0]);
            w_policy_a1[0][s] = w_policy_a1[0][s] - lr * adv[i] * probs[1];
        }
        else // a == 1
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
        // gradient step
        w_val[0][s] = w_val[0][s] - lr * diff;
        i = i + 1;
    }
}

// ------------------------------------------------------------
// Main driver
// ------------------------------------------------------------
int main()
{
    // seed randomness
    std::srand(static_cast<unsigned>(std::time(0)));

    // --------------------------------------------------------
    // Duplicate‑heavy predefined MDP
    // --------------------------------------------------------
    int num_states = 5; // increased state count with many duplicates
    // transition table: next_state[s][a] (all transitions lead to state 0)
    std::vector<std::vector<int> > nxt_state(num_states, std::vector<int>(2));
    int i = 0;
    while (i < num_states)
    {
        nxt_state[i][0] = 0;
        nxt_state[i][1] = 0;
        i = i + 1;
    }

    // reward table: reward[s][a] (all rewards are identical)
    std::vector<std::vector<float> > rew(num_states, std::vector<float>(2));
    i = 0;
    while (i < num_states)
    {
        rew[i][0] = 1.0f;
        rew[i][1] = 1.0f;
        i = i + 1;
    }

    // --------------------------------------------------------
    // Initialise policy and value parameters (identical values for all states)
    // --------------------------------------------------------
    std::vector<std::vector<float> > w_policy_a0(1, std::vector<float>(num_states));
    std::vector<std::vector<float> > w_policy_a1(1, std::vector<float>(num_states));
    std::vector<std::vector<float> > w_val(1, std::vector<float>(num_states));
    i = 0;
    while (i < num_states)
    {
        w_policy_a0[0][i] = 0.0f;
        w_policy_a1[0][i] = 0.0f;
        w_val[0][i]      = 0.0f;
        i = i + 1;
    }

    // --------------------------------------------------------
    // Training hyper‑parameters
    // --------------------------------------------------------
    int total_episodes = 20;
    int max_steps_per_ep = 5;
    float gamma = 0.9f;
    float lr_policy = 0.05f;
    float lr_value  = 0.1f;

    // --------------------------------------------------------
    // Training loop
    // --------------------------------------------------------
    int ep = 0;
    while (ep < total_episodes)
    {
        // containers for a single episode
        std::vector<int> ep_states;
        std::vector<int> ep_actions;
        std::vector<float> ep_rewards;

        // run episode
        run_episode(nxt_state, rew, w_policy_a0, w_policy_a1,
                    ep_states, ep_actions, ep_rewards,
                    max_steps_per_ep);

        // compute returns G_t
        std::vector<float> returns;
        compute_returns(ep_rewards, gamma, returns);

        // compute current state values V(s_t)
        std::vector<float> values;
        compute_values(ep_states, w_val, values);

        // compute advantages A_t
        std::vector<float> adv;
        compute_advantages(returns, values, adv);

        // update policy (actor)
        update_policy(ep_states, ep_actions, adv,
                      w_policy_a0, w_policy_a1, lr_policy);

        // update value function (critic)
        update_value(ep_states, returns, w_val, lr_value);

        ep = ep + 1;
    }

    // --------------------------------------------------------
    // Print final policy probabilities for each state
    // --------------------------------------------------------
    std::cout << "Final policy (probability of action 0) per state:\n";
    int s = 0;
    while (s < num_states)
    {
        float logit0 = w_policy_a0[0][s];
        float logit1 = w_policy_a1[0][s];
        std::vector<float> logits(2);
        logits[0] = logit0; logits[1] = logit1;
        std::vector<float> probs(2);
        softmax_pair(logits, probs);
        std::cout << "State " << s << ": " << probs[0] << "\n";
        s = s + 1;
    }

    // --------------------------------------------------------
    // Print final state values
    // --------------------------------------------------------
    std::cout << "\nFinal state values:\n";
    s = 0;
    while (s < num_states)
    {
        std::cout << "State " << s << ": " << w_val[0][s] << "\n";
        s = s + 1;
    }

    return 0;
}
