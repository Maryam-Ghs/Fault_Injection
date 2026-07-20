// A2C (Advantage Actor‑Critic) – version #2
// ------------------------------------------------------------
// This implementation uses only float and int, no const, no double.
// It generates a tiny deterministic environment internally,
// runs a few episodes of A2C and prints the learned parameters.

/* LLM input variant 2: small-diverse */

// ------------------------------------------------------------
#include <iostream>
#include <vector>
#include <cmath>    // for std::exp

// ------------------------------------------------------------
// Helper: sigmoid activation (reordered arithmetic)
float sigmoid(float x)
{
    // 1 / (1 + exp(-x))  ->  reordering: exp(-x) + 1, then reciprocal
    float ex = std::exp(-x);
    float denom = ex + 1.0f;
    float out = 1.0f / denom;
    return out;
}

// ------------------------------------------------------------
// Helper: dot product of two vectors (verbose)
float dot_product(const std::vector<float>& a, const std::vector<float>& b)
{
    float result = 0.0f;
    int idx = 0;
    while (idx < (int)a.size())
    {
        // result = result + a[idx] * b[idx];
        float prod = a[idx] * b[idx];
        result = result + prod;
        idx = idx + 1;
    }
    return result;
}

// ------------------------------------------------------------
// Helper: compute policy probability for action = 1
float policy_probability(const std::vector<float>& state,
                         const std::vector<float>& policy_w)
{
    float lin = dot_product(state, policy_w);
    float prob = sigmoid(lin);
    return prob;
}

// ------------------------------------------------------------
// Helper: compute state value estimate
float state_value(const std::vector<float>& state,
                  const std::vector<float>& value_w)
{
    float v = dot_product(state, value_w);
    return v;
}

// ------------------------------------------------------------
// Helper: sample binary action (0/1) using policy probability
int sample_action(float prob)
{
    // deterministic sampling for reproducibility:
    // if prob > 0.5 -> 1 else 0
    int act = 0;
    if (prob > 0.5f)
        act = 1;
    else
        act = 0;
    return act;
}

// ------------------------------------------------------------
// Helper: compute returns (discounted sum of rewards)
// reversed loop, reordered arithmetic: G = r + gamma * G_next
std::vector<float> compute_returns(const std::vector<float>& rewards,
                                   float gamma)
{
    int T = (int)rewards.size();
    std::vector<float> G(T, 0.0f);
    float next = 0.0f;
    int t = T - 1;
    while (t >= 0)
    {
        // G[t] = rewards[t] + gamma * next;
        float term = gamma * next;
        G[t] = rewards[t] + term;
        next = G[t];
        t = t - 1;
    }
    return G;
}

// ------------------------------------------------------------
// Helper: update policy and value weights using A2C gradients
void a2c_update(const std::vector<std::vector<float>>& trajectory_states,
                const std::vector<int>& trajectory_actions,
                const std::vector<float>& trajectory_returns,
                std::vector<float>& policy_w,
                std::vector<float>& value_w,
                float lr)
{
    int steps = (int)trajectory_states.size();
    int i = 0;
    while (i < steps)
    {
        const std::vector<float>& s = trajectory_states[i];
        int a = trajectory_actions[i];
        float G = trajectory_returns[i];

        // ----- value estimate and advantage -----
        float V = state_value(s, value_w);
        float adv = G - V;                     // advantage

        // ----- policy gradient -----
        float pi = policy_probability(s, policy_w);
        // gradient of logπ = (a - π) * s
        float diff = (float)a - pi;            // (a - π)
        int j = 0;
        while (j < (int)policy_w.size())
        {
            float grad = adv * diff * s[j];    // adv * (a-π) * s_j
            // policy_w = policy_w + lr * grad
            float update = lr * grad;
            policy_w[j] = policy_w[j] + update;
            j = j + 1;
        }

        // ----- value gradient (MSE) -----
        j = 0;
        while (j < (int)value_w.size())
        {
            float grad_v = -2.0f * adv * s[j]; // -2 * (G-V) * s_j
            float update_v = lr * grad_v;
            value_w[j] = value_w[j] + update_v;
            j = j + 1;
        }

        i = i + 1;
    }
}

// ------------------------------------------------------------
// Main entry point
int main()
{
    // --------------------------------------------------------
    // Hyper‑parameters (float only)
    float gamma = 0.8f;          // discount factor
    float lr = 0.10f;            // learning rate
    int episodes = 4;            // tiny number of episodes
    int state_dim = 4;           // dimension of one‑hot state

    // --------------------------------------------------------
    // Initialise policy and value weights with small numbers
    std::vector<float> policy_w(state_dim, 0.1f);
    std::vector<float> value_w(state_dim, 0.1f);

    // --------------------------------------------------------
    // Pre‑defined deterministic environment
    // Four one‑hot states, each with a fixed reward
    std::vector<std::vector<float>> all_states = {
        {1.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f}
    };
    std::vector<float> all_rewards = {0.5f, -0.2f, 1.0f, -0.5f};

    // --------------------------------------------------------
    // Run several episodes
    int ep = 0;
    while (ep < episodes)
    {
        // Containers for this episode
        std::vector<std::vector<float>> ep_states;
        std::vector<int> ep_actions;
        std::vector<float> ep_rewards;

        // Walk through the four states in order
        int step = 0;
        while (step < (int)all_states.size())
        {
            const std::vector<float>& cur_state = all_states[step];
            float prob = policy_probability(cur_state, policy_w);
            int act = sample_action(prob);
            float rew = all_rewards[step];

            // Store trajectory
            ep_states.push_back(cur_state);
            ep_actions.push_back(act);
            ep_rewards.push_back(rew);

            step = step + 1;
        }

        // Compute returns for the episode
        std::vector<float> ep_returns = compute_returns(ep_rewards, gamma);

        // Update networks with A2C rule
        a2c_update(ep_states, ep_actions, ep_returns,
                   policy_w, value_w, lr);

        ep = ep + 1;
    }

    // --------------------------------------------------------
    // Print learned parameters and a sample evaluation
    std::cout << "Learned policy weights:" << std::endl;
    int i = 0;
    while (i < (int)policy_w.size())
    {
        std::cout << "  w[" << i << "] = " << policy_w[i] << std::endl;
        i = i + 1;
    }

    std::cout << "Learned value weights:" << std::endl;
    i = 0;
    while (i < (int)value_w.size())
    {
        std::cout << "  v[" << i << "] = " << value_w[i] << std::endl;
        i = i + 1;
    }

    // Sample evaluation on each state
    std::cout << "Final policy probabilities (action=1):" << std::endl;
    i = 0;
    while (i < (int)all_states.size())
    {
        float p = policy_probability(all_states[i], policy_w);
        std::cout << "  State " << i << " -> prob = " << p << std::endl;
        i = i + 1;
    }

    return 0;
}
