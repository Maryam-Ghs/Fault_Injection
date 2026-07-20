/* LLM input variant 7: reverse-adversarial */
/*********************************************************************
 * A2C (Advantage Actor‑Critic) – version #7
 *
 *  - Only float and int are used (no double, long, unsigned, const)
 *  - All data are kept in stack‑allocated arrays
 *  - Helper functions split the algorithm into clear steps
 *  - Verbose, step‑by‑step style with direct arithmetic
 *  - Edge‑case heavy internal “input” (zero‑length episodes, extreme rewards)
 *********************************************************************/

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>        // for expf

/* ----------------------------------------------------------------- */
/*   Problem dimensions (stack allocation)                           */
/* ----------------------------------------------------------------- */
#define MAX_STATES   5
#define MAX_ACTIONS  3
#define MAX_EPISODES 4
#define MAX_STEPS    6

/* ----------------------------------------------------------------- */
/*   Utility: softmax – converts raw scores to a probability vector */
/* ----------------------------------------------------------------- */
void softmax(float raw[MAX_ACTIONS], float prob[MAX_ACTIONS])
{
    float maxv = raw[0];
    for (int i = 1; i < MAX_ACTIONS; ++i)
        if (raw[i] > maxv) maxv = raw[i];

    float sum = 0.0f;
    for (int i = 0; i < MAX_ACTIONS; ++i)
    {
        // shift by maxv for numeric stability
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
    float r = static_cast<float>(std::rand()) / RAND_MAX;
    float accum = 0.0f;
    for (int a = 0; a < MAX_ACTIONS; ++a)
    {
        accum += prob[a];
        if (r <= accum) return a;
    }
    return MAX_ACTIONS - 1; // fallback
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
    // gradient = advantage * (1 - prob) for taken action,
    //            -advantage * prob   for others
    for (int a = 0; a < MAX_ACTIONS; ++a)
    {
        float grad = 0.0f;
        if (a == action)
            grad = advantage * (1.0f - prob[a]);
        else
            grad = -advantage * prob[a];
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
    // simple linear value: V(s) = w[s]
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
    // Reverse‑order edge‑case: even episodes are full, odd are empty
    int steps = (episode_id % 2 == 0) ? MAX_STEPS : 0;

    float total_reward = 0.0f;
    int   cur_state = episode_id % MAX_STATES; // start state varies

    for (int t = 0; t < steps; ++t)
    {
        // 1. Compute raw scores and probabilities for current state
        float raw[MAX_ACTIONS];
        for (int a = 0; a < MAX_ACTIONS; ++a)
            raw[a] = policy_w[cur_state][a];
        float prob[MAX_ACTIONS];
        softmax(raw, prob);

        // 2. Sample an action
        int act = sample_action(prob);

        // 3. Generate a reward – reversed extreme values
        //    even timesteps give huge negative reward, odd give huge positive
        float reward = (t % 2 == 0) ? -1000.0f : 1000.0f;
        total_reward += reward;

        // 4. Observe next state (wrap around)
        int next_state = (cur_state + 1) % MAX_STATES;

        // 5. Compute value estimates
        float V_cur  = value_w[cur_state];
        float V_next = value_w[next_state];

        // 6. TD error δ = r + γ V(s') - V(s)
        //    Using γ = 0.9 (hard‑coded)
        float td_error = reward + 0.9f * V_next - V_cur;

        // 7. Advantage = TD error (since baseline = V(s))
        float adv = compute_advantage(reward, V_cur);

        // 8. Update actor and critic
        update_policy(policy_w, cur_state, act, adv, prob, lr_actor);
        update_value (value_w, cur_state, td_error, lr_critic);

        // 9. Move to next state
        cur_state = next_state;
    }

    // Print episode summary (verbose)
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
    // Fixed seed for deterministic behaviour
    std::srand(0);

    // 1. Initialise policy and value weights with descending deterministic numbers
    float policy_w[MAX_STATES][MAX_ACTIONS];
    float value_w [MAX_STATES];
    float base = 0.05f; // start slightly positive
    for (int s = 0; s < MAX_STATES; ++s)
    {
        for (int a = 0; a < MAX_ACTIONS; ++a)
            policy_w[s][a] = base - ((s * MAX_ACTIONS + a) * 0.001f);
        value_w[s] = base - (s * 0.001f);
    }

    // 2. Hyper‑parameters (chosen to be simple floats)
    float lr_actor  = 0.5f;   // larger learning rate for adversarial effect
    float lr_critic = 0.5f;

    // 3. Run a series of episodes (including edge‑case zero‑step episodes)
    for (int ep = 0; ep < MAX_EPISODES; ++ep)
        run_episode(policy_w, value_w, lr_actor, lr_critic, ep);

    // 4. Final policy display – probabilities per state
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

    // 5. Final value function display
    std::cout << "\nFinal state‑value estimates:\n";
    for (int s = 0; s < MAX_STATES; ++s)
        std::cout << "V(" << s << ") = " << value_w[s] << "\n";

    return 0;
}
