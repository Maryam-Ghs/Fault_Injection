#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

/* LLM input variant 6: ordered-structured */

int main()
{
    /* ---------- deterministic test data ---------- */
    int state_dim = 3;
    int act_dim   = 2;
    int steps     = 5;                     // length of the deterministic trajectory

    std::vector<std::vector<float>> traj_state(steps, std::vector<float>(state_dim));
    std::vector<int>               traj_action(steps);
    std::vector<float>             traj_reward(steps);
    std::vector<std::vector<float>> traj_next_state(steps, std::vector<float>(state_dim));

    // ordered, structured vectors
    traj_state[0] = {0.0f, 0.0f, 0.0f};
    traj_state[1] = {1.0f, 1.0f, 1.0f};
    traj_state[2] = {2.0f, 2.0f, 2.0f};
    traj_state[3] = {3.0f, 3.0f, 3.0f};
    traj_state[4] = {4.0f, 4.0f, 4.0f};

    // simple alternating action pattern
    traj_action[0] = 0;
    traj_action[1] = 1;
    traj_action[2] = 0;
    traj_action[3] = 1;
    traj_action[4] = 0;

    // monotonically increasing rewards
    traj_reward[0] = 0.1f;
    traj_reward[1] = 0.2f;
    traj_reward[2] = 0.3f;
    traj_reward[3] = 0.4f;
    traj_reward[4] = 0.5f;

    traj_next_state[0] = traj_state[1];
    traj_next_state[1] = traj_state[2];
    traj_next_state[2] = traj_state[3];
    traj_next_state[3] = traj_state[4];
    traj_next_state[4] = {0.0f, 0.0f, 0.0f};   // terminal

    /* ---------- model parameters (initialized) ---------- */
    std::vector<float> pol_w(state_dim * act_dim);   // policy linear weights
    std::vector<float> val_w(state_dim);             // value linear weights

    // simple deterministic init
    int idx = 0;
    while (idx < (int)pol_w.size())
    {
        pol_w[idx] = 0.1f * (idx + 1);
        ++idx;
    }
    idx = 0;
    while (idx < (int)val_w.size())
    {
        val_w[idx] = 0.05f * (idx + 1);
        ++idx;
    }

    /* ---------- helper lambdas ---------- */
    auto dot = [&](const std::vector<float>& a, const std::vector<float>& b) -> float
    {
        float res = 0.0f;
        int i = 0;
        while (i < (int)a.size())
        {
            res += a[i] * b[i];
            ++i;
        }
        return res;
    };

    auto softmax = [&](const std::vector<float>& logits) -> std::vector<float>
    {
        std::vector<float> probs(logits.size());
        float max_log = logits[0];
        int i = 1;
        while (i < (int)logits.size())
        {
            if (logits[i] > max_log) max_log = logits[i];
            ++i;
        }
        float sum = 0.0f;
        i = 0;
        while (i < (int)logits.size())
        {
            float e = static_cast<float>(std::exp(logits[i] - max_log));
            probs[i] = e;
            sum += e;
            ++i;
        }
        i = 0;
        while (i < (int)probs.size())
        {
            probs[i] /= sum;
            ++i;
        }
        return probs;
    };

    auto policy = [&](const std::vector<float>& s) -> std::vector<float>
    {
        std::vector<float> logits(act_dim);
        int a = 0;
        while (a < act_dim)
        {
            int off = a * state_dim;
            float l = 0.0f;
            int j = 0;
            while (j < state_dim)
            {
                l += pol_w[off + j] * s[j];
                ++j;
            }
            logits[a] = l;
            ++a;
        }
        return softmax(logits);
    };

    auto value = [&](const std::vector<float>& s) -> float
    {
        return dot(val_w, s);
    };

    /* ---------- A2C learning loop ---------- */
    float gamma   = 0.99f;
    float lr_pol  = 0.01f;
    float lr_val  = 0.02f;
    int   epochs  = 8;                     // version #8 as requested

    int ep = 0;
    while (ep < epochs)
    {
        // compute returns G_t for the whole trajectory (reverse pass)
        std::vector<float> returns(steps);
        float G = 0.0f;
        int t = steps - 1;
        while (t >= 0)
        {
            G = traj_reward[t] + gamma * G;
            returns[t] = G;
            --t;
        }

        // one pass over the trajectory
        int step = 0;
        while (step < steps)
        {
            const std::vector<float>& s   = traj_state[step];
            const std::vector<float>& ns  = traj_next_state[step];
            int a                         = traj_action[step];
            float Gt                      = returns[step];
            float V_s                     = value(s);
            float advantage               = Gt - V_s;

            // ----- policy gradient update -----
            std::vector<float> pi = policy(s);
            // gradient for linear softmax policy: (one_hot - pi) * advantage * s
            int a_idx = 0;
            while (a_idx < act_dim)
            {
                float coeff = ( (a_idx == a) ? 1.0f : 0.0f ) - pi[a_idx];
                int off = a_idx * state_dim;
                int j = 0;
                while (j < state_dim)
                {
                    pol_w[off + j] += lr_pol * advantage * coeff * s[j];
                    ++j;
                }
                ++a_idx;
            }

            // ----- value function update (MSE) -----
            int i = 0;
            while (i < state_dim)
            {
                val_w[i] += lr_val * advantage * s[i];
                ++i;
            }

            ++step;
        }

        ++ep;
    }

    /* ---------- output final parameters ---------- */
    std::cout << "Final policy weights (flattened):\n";
    int i = 0;
    while (i < (int)pol_w.size())
    {
        std::cout << pol_w[i] << ' ';
        ++i;
    }
    std::cout << "\nFinal value weights:\n";
    i = 0;
    while (i < (int)val_w.size())
    {
        std::cout << val_w[i] << ' ';
        ++i;
    }
    std::cout << std::endl;
    return 0;
}
