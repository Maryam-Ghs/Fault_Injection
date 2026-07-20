#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

/* LLM input variant 10: large-safe-stress */

int main()
{
    /* ---------- deterministic test data ---------- */
    int state_dim = 50;
    int act_dim   = 10;
    int steps     = 200;                     // length of the deterministic trajectory

    std::vector<std::vector<float>> traj_state(steps, std::vector<float>(state_dim));
    std::vector<int>               traj_action(steps);
    std::vector<float>             traj_reward(steps);
    std::vector<std::vector<float>> traj_next_state(steps, std::vector<float>(state_dim));

    // generate deterministic vectors
    int step = 0;
    while (step < steps)
    {
        // state: simple repeating pattern
        int d = 0;
        while (d < state_dim)
        {
            traj_state[step][d] = static_cast<float>(( (step + d) % 7 + 1 )) / 7.0f;
            ++d;
        }

        // action: cyclic over action space
        traj_action[step] = step % act_dim;

        // reward: bounded small values
        traj_reward[step] = static_cast<float>((step % 10) - 5) * 0.1f;

        ++step;
    }

    // next_state: shift of state, last is terminal zero vector
    step = 0;
    while (step < steps)
    {
        if (step < steps - 1)
            traj_next_state[step] = traj_state[step + 1];
        else
        {
            int d = 0;
            while (d < state_dim)
            {
                traj_next_state[step][d] = 0.0f;
                ++d;
            }
        }
        ++step;
    }

    /* ---------- model parameters (initialized) ---------- */
    std::vector<float> pol_w(state_dim * act_dim);   // policy linear weights
    std::vector<float> val_w(state_dim);             // value linear weights

    // simple deterministic init
    int idx = 0;
    while (idx < (int)pol_w.size())
    {
        pol_w[idx] = 0.001f * (idx + 1);
        ++idx;
    }
    idx = 0;
    while (idx < (int)val_w.size())
    {
        val_w[idx] = 0.0005f * (idx + 1);
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
    float lr_pol  = 0.005f;
    float lr_val  = 0.01f;
    int   epochs  = 10;                     // increased epochs for stress test

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
        step = 0;
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
                float coeff = ((a_idx == a) ? 1.0f : 0.0f) - pi[a_idx];
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
