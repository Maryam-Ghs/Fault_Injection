/*********************************************************************
 * A2C (Advantage Actor‑Critic) – version #7
 *
 *  - Only float and int are used (no double, long, unsigned, const)
 *  - All data are kept in stack‑allocated arrays
 *  - Helper functions split the algorithm into clear steps
 *  - Verbose, step‑by‑step style with direct arithmetic
 *  - Edge‑case heavy internal “input” (zero‑length episodes, extreme rewards)
 *
 * LLM input variant 9: medium-deterministic-random
 */

/* ----------------------------------------------------------------- */
/*   Problem dimensions (stack allocation)                           */
/* ----------------------------------------------------------------- */
#define MAX_STATES   7
#define MAX_ACTIONS  4
#define MAX_EPISODES 5
#define MAX_STEPS    8

#include <iostream>
#include <cstdlib>
#include <cmath>        // for expf

/* ----------------------------------------------------------------- */
/*   Deterministic pseudo‑random generator (LCG)                       */
/* ----------------------------------------------------------------- */
static unsigned int lcg_state = 123456789u;
float deterministic_rand_float()
{
    lcg_state = lcg_state * 1664525u + 1013904223u;
    return static_cast<float>((lcg_state >> 16) & 0xFFFFu) / 65535.0f; // [0,1]
}

/* ----------------------------------------------------------------- */
/*   Utility: softmax – converts raw scores to a probability vector   */
/* ----------------------------------------------------------------- */
void softmax(float raw[MAX_ACTIONS], float prob[MAX_ACTIONS])
{
    float maxv = raw[0];
    for (int i = 1; i < MAX_ACTIONS; ++i)
        if (raw[i] > maxv) maxv = raw[i];

    float sum = 0.0f;
    for (int i = 0; i < MAX_ACTIONS; ++i)
    {
        prob[i] = expf(raw[i] - maxv);
        sum += prob[i];
    }
    for (int i = 0; i < MAX_ACTIONS; ++i)
        prob[i] /= sum;
}

/* ----------------------------------------------------------------- */
/*   Sample an action according to a probability distribution         */
/* ----------------------------------------------------------------- */
int sample_action(const float prob[MAX_ACTIONS])
{
    float r = deterministic_rand_float();
    float accum = 0.0f;
    for (int a = 0; a < MAX_ACTIONS; ++a)
    {
        accum += prob[a];
        if (r <= accum) return a;
    }
    return MAX_ACTIONS - 1;
}

/* ----------------------------------------------------------------- */
/*   Compute advantage: A = R - V(s)                                  */
/* ----------------------------------------------------------------- */
float compute_advantage(float reward, float value)
{
    return reward - value;
}

/* ----------------------------------------------------------------- */
/*   Update policy (actor) weights – gradient ascent                  */
/* ----------------------------------------------------------------- */
void update_policy(
    float policy_w[MAX_STATES][MAX_ACTIONS],
    int   state,
    int   action,
    float advantage,
    float prob[MAX_ACTIONS],
    float lr)
{
    for (int a = 0; a < MAX_ACTIONS; ++a)
    {
        float grad = (a == action) ? advantage * (1.0f - prob[a])
                                   : -advantage * prob[a];
        policy_w[state][a] += lr * grad;
    }
}

/* ----------------------------------------------------------------- */
/*   Update value (critic) weights – gradient descent                 */
/* ----------------------------------------------------------------- */
void update_value(
    float value_w[MAX_STATES],
    int   state,
    float td_error,
    float lr)
{
    value_w[state] += lr * td_error;
}

/* ----------------------------------------------------------------- */
/*   Run a single episode, returning cumulative reward                */
/* ----------------------------------------------------------------- */
float run_episode(
    float policy_w[MAX_STATES][MAX_ACTIONS],
    float value_w[MAX_STATES],
    float lr_actor,
    float lr_critic,
    int   episode_id)
{
    /* Edge‑case handling: some episodes have zero steps */
    int steps = (episode_id % 3 == 0) ? 0 : MAX_STEPS;

    float total_reward = 0.0f;
    int   cur_state = (episode_id * 2) % MAX_STATES; // varied start state

    for (int t = 0; t < steps; ++t)
    {
        float raw[MAX_ACTIONS];
        for (int a = 0; a < MAX_ACTIONS; ++a)
            raw[a] = policy_w[cur_state][a];
        float prob[MAX_ACTIONS];
        softmax(raw, prob);

        int act = sample_action(prob);

        float reward = (t % 2 == 0) ? 1000.0f : -1000.0f;
        total_reward += reward;

        int next_state = (cur_state + 1) % MAX_STATES;

        float V_cur  = value_w[cur_state];
        float V_next = value_w[next_state];

        float td_error = reward + 0.9f * V_next - V_cur;
        float adv = compute_advantage(reward, V_cur);

        update_policy(policy_w, cur_state, act, adv, prob, lr_actor);
        update_value (value_w, cur_state, td_error, lr_critic);

        cur_state = next_state;
    }

    std::cout << "Episode " << episode_id
              << " | steps " << steps
              << " | cumulative reward " << total_reward << "\n";

    return total_reward;
}

/* ----------------------------------------------------------------- */
/*   Main – orchestrates multiple episodes                           */
/* ----------------------------------------------------------------- */
int main()
{
    /* Initialise policy and value weights with deterministic pattern */
    float policy_w[MAX_STATES][MAX_ACTIONS];
    float value_w [MAX_STATES];
    for (int s = 0; s < MAX_STATES; ++s)
    {
        for (int a = 0; a < MAX_ACTIONS; ++a)
            policy_w[s][a] = (deterministic_rand_float() - 0.5f) * 0.1f;
        value_w[s] = (deterministic_rand_float() - 0.5f) * 0.1f;
    }

    float lr_actor  = 0.01f;
    float lr_critic = 0.02f;

    for (int ep = 0; ep < MAX_EPISODES; ++ep)
        run_episode(policy_w, value_w, lr_actor, lr_critic, ep);

    std::cout << "\nFinal policy (probabilities per state):\n";
    for (int s = 0; s < MAX_STATES; ++s)
    {
        float raw[MAX_ACTIONS];
        for (int a = 0; a < MAX_ACTIONS; ++a)
            raw[a] = policy_w[s][a];
        float prob[MAX_ACTIONS];
        softmax(raw, prob);
        std::cout << "State " << s << ": ";
        for (int a = 0; a < MAX_ACTIONS; ++a)
            std::cout << "A" << a << "=" << prob[a] << " ";
        std::cout << "\n";
    }

    std::cout << "\nFinal state‑value estimates:\n";
    for (int s = 0; s < MAX_STATES; ++s)
        std::cout << "V(" << s << ") = " << value_w[s] << "\n";

    return 0;
}
