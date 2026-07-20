#include <iostream>
#include <cmath>
#include <iomanip>

/* LLM input variant 9: medium-deterministic-random */

// ------------------------------------------------------------------
// Configuration (all integers and floats only)
// ------------------------------------------------------------------
int NUM_STATES = 7;          // number of distinct states
int NUM_ACTIONS = 3;         // number of possible actions
int EPISODES = 8;            // how many episodes to run
int STEPS_PER_EP = 6;        // fixed length of each episode
float GAMMA = 0.85f;         // discount factor
float LR_POLICY = 0.04f;     // learning rate for policy network
float LR_VALUE = 0.04f;      // learning rate for value network

// ------------------------------------------------------------------
// Helper: initialise deterministic weights (no random)
// ------------------------------------------------------------------
void initialise_weights(float w_policy[][NUM_ACTIONS], float w_value[])
{
    // deterministic pseudo‑random pattern
    for (int i = 0; i < NUM_STATES; ++i)
    {
        for (int j = 0; j < NUM_ACTIONS; ++j)
        {
            // example pattern producing values like 0.07, 0.14, 0.21 …
            w_policy[i][j] = 0.07f * ((i + j * 2 + 1) % 10 + 1);
        }
        w_value[i] = 0.03f * ((i * 3) % 10 + 1); // e.g. 0.03, 0.06, …
    }
}

// ------------------------------------------------------------------
// Helper: build one‑hot state vector from index
// ------------------------------------------------------------------
void make_state_vector(int s_idx, float s_vec[])
{
    for (int i = 0; i < NUM_STATES; ++i)
        s_vec[i] = 0.0f;
    s_vec[s_idx] = 1.0f;
}

// ------------------------------------------------------------------
// Helper: compute raw logits = state * weight matrix
// ------------------------------------------------------------------
void compute_logits(const float s_vec[], const float w_policy[][NUM_ACTIONS],
                    float logits[])
{
    for (int a = 0; a < NUM_ACTIONS; ++a)
    {
        float acc = 0.0f;
        for (int i = 0; i < NUM_STATES; ++i)
        {
            acc += s_vec[i] * w_policy[i][a];
        }
        logits[a] = acc;
    }
}

// ------------------------------------------------------------------
// Helper: soft‑max to obtain action probabilities
// ------------------------------------------------------------------
void softmax(const float logits[], float probs[])
{
    // compute exponentials safely
    float max_logit = logits[0];
    for (int i = 1; i < NUM_ACTIONS; ++i)
        if (logits[i] > max_logit) max_logit = logits[i];

    float sum = 0.0f;
    float exp_vals[NUM_ACTIONS];
    for (int i = 0; i < NUM_ACTIONS; ++i)
    {
        exp_vals[i] = std::exp(logits[i] - max_logit); // improve stability
        sum += exp_vals[i];
    }
    for (int i = 0; i < NUM_ACTIONS; ++i)
        probs[i] = exp_vals[i] / sum;
}

// ------------------------------------------------------------------
// Helper: compute value V(s) = state · w_value
// ------------------------------------------------------------------
void compute_value(const float s_vec[], const float w_value[], float &val)
{
    float total = 0.0f;
    for (int i = 0; i < NUM_STATES; ++i)
        total += s_vec[i] * w_value[i];
    val = total;
}

// ------------------------------------------------------------------
// Helper: deterministic environment dynamics
// ------------------------------------------------------------------
void step_environment(int cur_state, int act, int &next_state, float &rew)
{
    // next state is (cur + act + 1) modulo number of states
    next_state = (cur_state + act + 1) % NUM_STATES;

    // reward is 1 only for a specific transition, otherwise 0
    if (cur_state == 1 && act == 2)
        rew = 1.0f;
    else
        rew = 0.0f;
}

// ------------------------------------------------------------------
// Helper: policy weight update using advantage
// ------------------------------------------------------------------
void update_policy(float w_policy[][NUM_ACTIONS], const float s_vec[],
                  const float probs[], int act_taken, float adv, float lr)
{
    for (int a = 0; a < NUM_ACTIONS; ++a)
    {
        float indicator = (a == act_taken) ? 1.0f : 0.0f;
        float diff = indicator - probs[a];
        for (int i = 0; i < NUM_STATES; ++i)
        {
            float grad = diff * s_vec[i];
            w_policy[i][a] += lr * adv * grad;
        }
    }
}

// ------------------------------------------------------------------
// Helper: value weight update using advantage
// ------------------------------------------------------------------
void update_value(float w_value[], const float s_vec[], float adv, float lr_v)
{
    for (int i = 0; i < NUM_STATES; ++i)
    {
        w_value[i] += lr_v * adv * s_vec[i];
    }
}

// ------------------------------------------------------------------
// Main driver
// ------------------------------------------------------------------
int main()
{
    // ----------------------------------------------------------------
    // 1. Allocate weight matrices on the stack
    // ----------------------------------------------------------------
    float w_policy[NUM_STATES][NUM_ACTIONS];
    float w_value[NUM_STATES];

    // ----------------------------------------------------------------
    // 2. Initialise them with deterministic numbers
    // ----------------------------------------------------------------
    initialise_weights(w_policy, w_value);

    // ----------------------------------------------------------------
    // 3. Run deterministic episodes
    // ----------------------------------------------------------------
    int episode = 0;
    while (episode < EPISODES)
    {
        int cur_state = 0;
        int step_cnt = 0;

        int state_seq[STEPS_PER_EP];
        int action_seq[STEPS_PER_EP];
        float reward_seq[STEPS_PER_EP];
        float value_seq[STEPS_PER_EP];
        float prob_seq[STEPS_PER_EP][NUM_ACTIONS];

        while (step_cnt < STEPS_PER_EP)
        {
            // a) build one‑hot representation of current state
            float s_vec[NUM_STATES];
            make_state_vector(cur_state, s_vec);

            // b) forward pass: compute policy and value
            float logits[NUM_ACTIONS];
            compute_logits(s_vec, w_policy, logits);

            float probs[NUM_ACTIONS];
            softmax(logits, probs);

            float val;
            compute_value(s_vec, w_value, val);

            // c) deterministic action selection (argmax)
            int chosen_action = 0;
            for (int a = 1; a < NUM_ACTIONS; ++a)
                if (probs[a] > probs[chosen_action])
                    chosen_action = a;

            // d) store trajectory data
            state_seq[step_cnt] = cur_state;
            action_seq[step_cnt] = chosen_action;
            reward_seq[step_cnt] = 0.0f; // placeholder
            value_seq[step_cnt] = val;
            for (int a = 0; a < NUM_ACTIONS; ++a)
                prob_seq[step_cnt][a] = probs[a];

            // e) environment step
            int nxt_state;
            float rew;
            step_environment(cur_state, chosen_action, nxt_state, rew);
            reward_seq[step_cnt] = rew;

            cur_state = nxt_state;
            ++step_cnt;
        }

        // ----------------------------------------------------------------
        // 4. Back‑propagate advantages and update parameters
        // ----------------------------------------------------------------
        int back_idx = STEPS_PER_EP - 1;
        while (back_idx >= 0)
        {
            int s_idx = state_seq[back_idx];
            float s_vec[NUM_STATES];
            make_state_vector(s_idx, s_vec);
            float v_curr = value_seq[back_idx];

            float v_next = 0.0f;
            if (back_idx < STEPS_PER_EP - 1)
                v_next = value_seq[back_idx + 1];

            float adv = reward_seq[back_idx] + GAMMA * v_next - v_curr;

            int a_taken = action_seq[back_idx];
            float probs[NUM_ACTIONS];
            for (int a = 0; a < NUM_ACTIONS; ++a)
                probs[a] = prob_seq[back_idx][a];

            update_policy(w_policy, s_vec, probs, a_taken, adv, LR_POLICY);
            update_value(w_value, s_vec, adv, LR_VALUE);

            --back_idx;
        }

        ++episode;
    }

    // ----------------------------------------------------------------
    // 5. Print final policy probabilities and value estimates
    // ----------------------------------------------------------------
    std::cout << std::fixed << std::setprecision(4);
    for (int s = 0; s < NUM_STATES; ++s)
    {
        float s_vec[NUM_STATES];
        make_state_vector(s, s_vec);

        float logits[NUM_ACTIONS];
        compute_logits(s_vec, w_policy, logits);
        float probs[NUM_ACTIONS];
        softmax(logits, probs);

        float val;
        compute_value(s_vec, w_value, val);

        std::cout << "State " << s << ":\n";
        for (int a = 0; a < NUM_ACTIONS; ++a)
        {
            std::cout << "  Policy -> Action" << a << ": " << probs[a];
            if (a < NUM_ACTIONS - 1) std::cout << " ,";
        }
        std::cout << "\n";
        std::cout << "  Value  -> " << val << "\n";
    }

    return 0;
}
