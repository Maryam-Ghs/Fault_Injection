#include <iostream>
#include <cmath>

/* LLM input variant 3: zeros-and-ones */

int main()
{
    // ---------- hyper‑parameters ----------
    int    max_epi      = 1;      // number of episodes
    int    max_step     = 1;      // steps per episode
    int    buf_cap      = 1;      // replay buffer capacity
    int    batch_sz     = 1;      // mini‑batch size
    float  gamma        = 1.0f;   // discount factor (identity)
    float  lr_actor     = 1.0f;   // learning rate for actor (identity)
    float  lr_critic    = 1.0f;   // learning rate for critic (identity)

    // ---------- problem dimensions ----------
    const int state_dim  = 3;      // dimension of state vector
    const int action_num = 2;      // number of discrete actions

    // ---------- linear function approximators ----------
    // actor: state_dim × action_num matrix, stored row‑major
    float w_policy[state_dim * action_num];
    // critic: state_dim vector
    float w_value[state_dim];

    // initialise weights to zeros, then set identity‑like ones
    int i = 0;
    while (i < state_dim * action_num) { w_policy[i] = 0.0f; ++i; }
    i = 0;
    while (i < state_dim)               { w_value[i]  = 0.0f; ++i; }

    // set a few policy weights to 1 (identity‑like)
    w_policy[0] = 1.0f; // row0, col0
    w_policy[3] = 1.0f; // row1, col1

    // ---------- experience replay buffer ----------
    float buf_state[buf_cap][state_dim];
    int   buf_act  [buf_cap];
    float buf_rew  [buf_cap];
    float buf_next [buf_cap][state_dim];
    int   buf_done [buf_cap];
    int   buf_size = 0;
    int   buf_ptr  = 0;

    // ---------- training loop ----------
    int epi = 0;
    while (epi < max_epi)
    {
        // start state = [0, 1, 2] (deterministic, but zeros‑heavy)
        float cur_state[state_dim];
        cur_state[0] = float(epi);
        cur_state[1] = float(epi + 1);
        cur_state[2] = float(epi + 2);

        int step = 0;
        while (step < max_step)
        {
            // ----- compute policy logits -----
            float logits[action_num];
            int a = 0;
            while (a < action_num)
            {
                // dot(state, w_policy column a)
                float dot = 0.0f;
                int s = 0;
                while (s < state_dim)
                {
                    dot += cur_state[s] * w_policy[s * action_num + a];
                    ++s;
                }
                logits[a] = dot;
                ++a;
            }

            // ----- softmax to obtain probabilities -----
            float max_log = logits[0];
            a = 1;
            while (a < action_num)
            {
                if (logits[a] > max_log) max_log = logits[a];
                ++a;
            }

            float sum_exp = 0.0f;
            a = 0;
            while (a < action_num)
            {
                logits[a] = std::exp(logits[a] - max_log);
                sum_exp += logits[a];
                ++a;
            }

            float probs[action_num];
            a = 0;
            while (a < action_num)
            {
                probs[a] = logits[a] / sum_exp;
                ++a;
            }

            // ----- select action (deterministic argmax) -----
            int act = 0;
            float best = probs[0];
            a = 1;
            while (a < action_num)
            {
                if (probs[a] > best) { best = probs[a]; act = a; }
                ++a;
            }

            // ----- deterministic reward and next state -----
            // reward = <cur_state, [1,2,3]> - act
            float reward = cur_state[0] * 1.0f + cur_state[1] * 2.0f + cur_state[2] * 3.0f - float(act);
            // next_state = cur_state + action_vector (action 0 -> [1,0,0], action 1 -> [0,1,0])
            float nxt_state[state_dim];
            nxt_state[0] = cur_state[0] + (act == 0 ? 1.0f : 0.0f);
            nxt_state[1] = cur_state[1] + (act == 1 ? 1.0f : 0.0f);
            nxt_state[2] = cur_state[2];                // unchanged

            int done = (step == max_step - 1) ? 1 : 0;

            // ----- store experience in buffer -----
            int idx = buf_ptr;
            int j = 0;
            while (j < state_dim) { buf_state[idx][j] = cur_state[j]; ++j; }
            buf_act[idx]   = act;
            buf_rew[idx]   = reward;
            j = 0;
            while (j < state_dim) { buf_next[idx][j] = nxt_state[j]; ++j; }
            buf_done[idx]  = done;

            ++buf_ptr;
            if (buf_ptr == buf_cap) buf_ptr = 0;
            if (buf_size < buf_cap) ++buf_size;

            // ----- move to next step -----
            int s = 0;
            while (s < state_dim) { cur_state[s] = nxt_state[s]; ++s; }
            ++step;

            // ----- perform learning from a mini‑batch -----
            if (buf_size >= batch_sz)
            {
                int b = 0;
                while (b < batch_sz)
                {
                    int sample = b; // deterministic sampling: first batch_sz entries

                    // ----- unpack one experience -----
                    float s_vec[state_dim];
                    int   a_sel;
                    float r_val;
                    float ns_vec[state_dim];
                    int   term;

                    int k = 0;
                    while (k < state_dim) { s_vec[k]  = buf_state[sample][k]; ++k; }
                    a_sel = buf_act[sample];
                    r_val = buf_rew[sample];
                    k = 0;
                    while (k < state_dim) { ns_vec[k] = buf_next[sample][k]; ++k; }
                    term = buf_done[sample];

                    // ----- compute V(s) and V(s') -----
                    float v_cur = 0.0f;
                    float v_nxt = 0.0f;
                    k = 0;
                    while (k < state_dim)
                    {
                        v_cur += w_value[k] * s_vec[k];
                        v_nxt += w_value[k] * ns_vec[k];
                        ++k;
                    }
                    if (term) v_nxt = 0.0f; // zero for terminal state

                    // ----- TD error -----
                    float delta = r_val + gamma * v_nxt - v_cur;

                    // ----- critic update: w_value += lr_critic * delta * s_vec -----
                    k = 0;
                    while (k < state_dim)
                    {
                        w_value[k] += lr_critic * delta * s_vec[k];
                        ++k;
                    }

                    // ----- actor update -----
                    // recompute policy probabilities for this state
                    float logit_a[action_num];
                    int aa = 0;
                    while (aa < action_num)
                    {
                        float dot = 0.0f;
                        int ss = 0;
                        while (ss < state_dim)
                        {
                            dot += s_vec[ss] * w_policy[ss * action_num + aa];
                            ++ss;
                        }
                        logit_a[aa] = dot;
                        ++aa;
                    }

                    // softmax
                    float max_l = logit_a[0];
                    aa = 1;
                    while (aa < action_num)
                    {
                        if (logit_a[aa] > max_l) max_l = logit_a[aa];
                        ++aa;
                    }

                    float sum_e = 0.0f;
                    aa = 0;
                    while (aa < action_num)
                    {
                        logit_a[aa] = std::exp(logit_a[aa] - max_l);
                        sum_e += logit_a[aa];
                        ++aa;
                    }

                    float pi[action_num];
                    aa = 0;
                    while (aa < action_num)
                    {
                        pi[aa] = logit_a[aa] / sum_e;
                        ++aa;
                    }

                    // gradient ascent on expected return: w += lr_actor * delta * (∂logπ/∂w)
                    // ∂logπ/∂w for linear softmax: (indicator - pi) * s_vec
                    aa = 0;
                    while (aa < action_num)
                    {
                        float factor = (aa == a_sel ? 1.0f - pi[aa] : -pi[aa]);
                        int ss = 0;
                        while (ss < state_dim)
                        {
                            w_policy[ss * action_num + aa] += lr_actor * delta * factor * s_vec[ss];
                            ++ss;
                        }
                        ++aa;
                    }

                    ++b;
                }
            }
        } // end of steps

        ++epi;
    } // end of episodes

    // ---------- print learned parameters ----------
    std::cout << "Final critic weights (w_value):\n";
    int d = 0;
    while (d < state_dim)
    {
        std::cout << w_value[d] << " ";
        ++d;
    }
    std::cout << "\n\nFinal actor weights (w_policy):\n";
    int row = 0;
    while (row < state_dim)
    {
        int col = 0;
        while (col < action_num)
        {
            std::cout << w_policy[row * action_num + col] << " ";
            ++col;
        }
        std::cout << "\n";
        ++row;
    }

    return 0;
}
