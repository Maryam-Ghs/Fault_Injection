#include <iostream>
#include <cmath>

/* LLM input variant 6: ordered-structured */

int main()
{
    // ---------- hyper‑parameters ----------
    int    max_epi      = 5;      // number of episodes
    int    max_step     = 6;      // steps per episode
    int    buf_cap      = 30;     // replay buffer capacity
    int    batch_sz     = 5;      // mini‑batch size
    float  gamma        = 0.9f;   // discount factor
    float  lr_actor     = 0.01f;  // learning rate for actor
    float  lr_critic    = 0.02f;  // learning rate for critic

    // ---------- problem dimensions ----------
    const int state_dim  = 3;      // dimension of state vector
    const int action_num = 2;      // number of discrete actions

    // ---------- linear function approximators ----------
    // actor: state_dim × action_num matrix, stored row‑major
    float w_policy[state_dim * action_num];
    // critic: state_dim vector
    float w_value[state_dim];

    // initialise weights to a simple increasing pattern (highly structured)
    for (int i = 0; i < state_dim * action_num; ++i) w_policy[i] = 0.01f * (i + 1);
    for (int i = 0; i < state_dim; ++i)               w_value[i]  = 0.01f * (i + 1);

    // ---------- experience replay buffer ----------
    float buf_state[buf_cap][state_dim];
    int   buf_act  [buf_cap];
    float buf_rew  [buf_cap];
    float buf_next[buf_cap][state_dim];
    int   buf_done [buf_cap];
    int   buf_size = 0;
    int   buf_ptr  = 0;

    // ---------- deterministic, highly structured test vectors ----------
    // start state = [epi, epi, epi]  (regular, sorted)
    // reward = dot(state, [1,2,3]) - action
    // done = (step == max_step-1)

    // ---------- training loop ----------
    for (int epi = 0; epi < max_epi; ++epi)
    {
        float cur_state[state_dim];
        cur_state[0] = static_cast<float>(epi);
        cur_state[1] = static_cast<float>(epi);
        cur_state[2] = static_cast<float>(epi);

        for (int step = 0; step < max_step; ++step)
        {
            // ----- compute policy logits -----
            float logits[action_num];
            for (int a = 0; a < action_num; ++a)
            {
                float dot = 0.0f;
                for (int s = 0; s < state_dim; ++s)
                    dot += cur_state[s] * w_policy[s * action_num + a];
                logits[a] = dot;
            }

            // ----- softmax to obtain probabilities -----
            float max_log = logits[0];
            for (int a = 1; a < action_num; ++a)
                if (logits[a] > max_log) max_log = logits[a];

            float sum_exp = 0.0f;
            for (int a = 0; a < action_num; ++a)
            {
                logits[a] = std::exp(logits[a] - max_log);
                sum_exp += logits[a];
            }

            float probs[action_num];
            for (int a = 0; a < action_num; ++a)
                probs[a] = logits[a] / sum_exp;

            // ----- select action (deterministic argmax) -----
            int act = 0;
            float best = probs[0];
            for (int a = 1; a < action_num; ++a)
                if (probs[a] > best) { best = probs[a]; act = a; }

            // ----- deterministic reward and next state -----
            float reward = cur_state[0] * 1.0f + cur_state[1] * 2.0f + cur_state[2] * 3.0f - static_cast<float>(act);
            float nxt_state[state_dim];
            // action 0 -> increment first dimension, action 1 -> increment second dimension
            nxt_state[0] = cur_state[0] + (act == 0 ? 1.0f : 0.0f);
            nxt_state[1] = cur_state[1] + (act == 1 ? 1.0f : 0.0f);
            nxt_state[2] = cur_state[2];                // unchanged

            int done = (step == max_step - 1) ? 1 : 0;

            // ----- store experience in buffer -----
            int idx = buf_ptr;
            for (int j = 0; j < state_dim; ++j) buf_state[idx][j] = cur_state[j];
            buf_act[idx]   = act;
            buf_rew[idx]   = reward;
            for (int j = 0; j < state_dim; ++j) buf_next[idx][j] = nxt_state[j];
            buf_done[idx]  = done;

            ++buf_ptr;
            if (buf_ptr == buf_cap) buf_ptr = 0;
            if (buf_size < buf_cap) ++buf_size;

            // ----- move to next step -----
            for (int s = 0; s < state_dim; ++s) cur_state[s] = nxt_state[s];

            // ----- perform learning from a mini‑batch -----
            if (buf_size >= batch_sz)
            {
                for (int b = 0; b < batch_sz; ++b)
                {
                    int sample = b; // deterministic sampling: first batch_sz entries

                    // ----- unpack one experience -----
                    float s_vec[state_dim];
                    int   a_sel;
                    float r_val;
                    float ns_vec[state_dim];
                    int   term;

                    for (int k = 0; k < state_dim; ++k) s_vec[k] = buf_state[sample][k];
                    a_sel = buf_act[sample];
                    r_val = buf_rew[sample];
                    for (int k = 0; k < state_dim; ++k) ns_vec[k] = buf_next[sample][k];
                    term = buf_done[sample];

                    // ----- compute V(s) and V(s') -----
                    float v_cur = 0.0f;
                    float v_nxt = 0.0f;
                    for (int k = 0; k < state_dim; ++k)
                    {
                        v_cur += w_value[k] * s_vec[k];
                        v_nxt += w_value[k] * ns_vec[k];
                    }
                    if (term) v_nxt = 0.0f; // zero for terminal state

                    // ----- TD error -----
                    float delta = r_val + gamma * v_nxt - v_cur;

                    // ----- critic update -----
                    for (int k = 0; k < state_dim; ++k)
                        w_value[k] += lr_critic * delta * s_vec[k];

                    // ----- actor update -----
                    // recompute policy probabilities for this state
                    float logit_a[action_num];
                    for (int aa = 0; aa < action_num; ++aa)
                    {
                        float dot = 0.0f;
                        for (int ss = 0; ss < state_dim; ++ss)
                            dot += s_vec[ss] * w_policy[ss * action_num + aa];
                        logit_a[aa] = dot;
                    }

                    // softmax
                    float max_l = logit_a[0];
                    for (int aa = 1; aa < action_num; ++aa)
                        if (logit_a[aa] > max_l) max_l = logit_a[aa];

                    float sum_e = 0.0f;
                    for (int aa = 0; aa < action_num; ++aa)
                    {
                        logit_a[aa] = std::exp(logit_a[aa] - max_l);
                        sum_e += logit_a[aa];
                    }

                    float pi[action_num];
                    for (int aa = 0; aa < action_num; ++aa)
                        pi[aa] = logit_a[aa] / sum_e;

                    // gradient ascent on expected return
                    for (int aa = 0; aa < action_num; ++aa)
                    {
                        float factor = (aa == a_sel ? 1.0f - pi[aa] : -pi[aa]);
                        for (int ss = 0; ss < state_dim; ++ss)
                            w_policy[ss * action_num + aa] += lr_actor * delta * factor * s_vec[ss];
                    }
                }
            }
        } // end of steps
    } // end of episodes

    // ---------- print learned parameters ----------
    std::cout << "Final critic weights (w_value):\n";
    for (int d = 0; d < state_dim; ++d)
        std::cout << w_value[d] << " ";
    std::cout << "\n\nFinal actor weights (w_policy):\n";
    for (int row = 0; row < state_dim; ++row)
    {
        for (int col = 0; col < action_num; ++col)
            std::cout << w_policy[row * action_num + col] << " ";
        std::cout << "\n";
    }

    return 0;
}
