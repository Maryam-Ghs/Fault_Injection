#include <iostream>
#include <cmath>
#include <iomanip>

/* LLM input variant 6: ordered-structured */

// ------------------------------------------------------------------
// Configuration (all integers and floats only)
// ------------------------------------------------------------------
const int NUM_STATES = 5;          // number of distinct states
const int NUM_ACTIONS = 4;         // number of possible actions
const int EPISODES = 3;            // how many episodes to run
const int STEPS_PER_EP = 6;        // fixed length of each episode
const float GAMMA = 0.9f;          // discount factor
const float LR_POLICY = 0.05f;     // learning rate for policy network
const float LR_VALUE = 0.05f;      // learning rate for value network

// ------------------------------------------------------------------
// Helper: initialise deterministic weights (no random)
// ------------------------------------------------------------------
void initialise_weights(float w_policy[NUM_STATES][NUM_ACTIONS], float w_value[NUM_STATES])
{
    // symmetric pattern: diagonal higher, off‑diagonal lower
    for (int i = 0; i < NUM_STATES; ++i)
    {
        for (int j = 0; j < NUM_ACTIONS; ++j)
        {
            w_policy[i][j] = (i == j ? 0.2f : 0.1f);
        }
        w_value[i] = 0.05f * (i + 1);  // 0.05, 0.10, 0.15, ...
    }
}

// ------------------------------------------------------------------
// Helper: build one‑hot state vector from index
// ------------------------------------------------------------------
void make_state_vector(int s_idx, float s_vec[NUM_STATES])
{
    for (int i = 0; i < NUM_STATES; ++i)
        s_vec[i] = 0.0f;
    s_vec[s_idx] = 1.0f;
}

// ------------------------------------------------------------------
// Helper: compute raw logits = state * weight matrix
// ------------------------------------------------------------------
void compute_logits(const float s_vec[NUM_STATES], const float w_policy[NUM_STATES][NUM_ACTIONS],
                    float logits[NUM_ACTIONS])
{
    for (int a = 0; a < NUM_ACTIONS; ++a)
    {
        float acc = 0.0f;
        for (int i = 0; i < NUM_STATES; ++i)
        {
            float prod = s_vec[i] * w_policy[i][a];
            acc = acc + prod;
        }
        logits[a] = acc;
    }
}

// ------------------------------------------------------------------
// Helper: soft‑max to obtain action probabilities
// ------------------------------------------------------------------
void softmax(const float logits[NUM_ACTIONS], float probs[NUM_ACTIONS])
{
    // exponentials
    float exps[NUM_ACTIONS];
    float sum = 0.0f;
    for (int i = 0; i < NUM_ACTIONS; ++i)
    {
        exps[i] = std::exp(logits[i]);
        sum += exps[i];
    }
    // probabilities
    for (int i = 0; i < NUM_ACTIONS; ++i)
        probs[i] = exps[i] / sum;
}

// ------------------------------------------------------------------
// Helper: compute value V(s) = state · w_value
// ------------------------------------------------------------------
void compute_value(const float s_vec[NUM_STATES], const float w_value[NUM_STATES],
                   float &val)
{
    float total = 0.0f;
    for (int i = 0; i < NUM_STATES; ++i)
    {
        float prod = s_vec[i] * w_value[i];
        total = total + prod;
    }
    val = total;
}

// ------------------------------------------------------------------
// Helper: deterministic environment dynamics
// ------------------------------------------------------------------
void step_environment(int cur_state, int act, int &next_state,
                      float &rew)
{
    // next state is (cur + act) modulo number of states
    next_state = (cur_state + act) % NUM_STATES;

    // reward is 1 only for a specific transition, otherwise 0
    if (cur_state == 0 && act == 1)
        rew = 1.0f;
    else
        rew = 0.0f;
}

// ------------------------------------------------------------------
// Helper: policy weight update using advantage
// ------------------------------------------------------------------
void update_policy(float w_policy[NUM_STATES][NUM_ACTIONS], const float s_vec[NUM_STATES],
                   const float probs[NUM_ACTIONS], int act_taken, float adv,
                   float lr)
{
    for (int a = 0; a < NUM_ACTIONS; ++a)
    {
        // gradient = (indicator - prob) * state
        float indicator = (a == act_taken) ? 1.0f : 0.0f;
        float diff = indicator - probs[a];

        for (int i = 0; i < NUM_STATES; ++i)
        {
            float grad = diff * s_vec[i];
            float delta = lr * adv * grad;
            w_policy[i][a] = w_policy[i][a] + delta;
        }
    }
}

// ------------------------------------------------------------------
// Helper: value weight update using advantage
// ------------------------------------------------------------------
void update_value(float w_value[NUM_STATES], const float s_vec[NUM_STATES],
                  float adv, float lr_v)
{
    for (int i = 0; i < NUM_STATES; ++i)
    {
        float grad = s_vec[i];
        float delta = lr_v * adv * grad;
        w_value[i] = w_value[i] + delta;
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
        // start each episode from state 0
        int cur_state = 0;
        int step_cnt = 0;

        // temporary storage for a trajectory (max length known)
        int state_seq[STEPS_PER_EP];
        int action_seq[STEPS_PER_EP];
        float reward_seq[STEPS_PER_EP];
        float value_seq[STEPS_PER_EP];
        float prob_seq[STEPS_PER_EP][NUM_ACTIONS];

        while (step_cnt < STEPS_PER_EP)
        {
            // --------------------------------------------------------
            // a) build one‑hot representation of current state
            // --------------------------------------------------------
            float s_vec[NUM_STATES];
            make_state_vector(cur_state, s_vec);

            // --------------------------------------------------------
            // b) forward pass: compute policy and value
            // --------------------------------------------------------
            float logits[NUM_ACTIONS];
            compute_logits(s_vec, w_policy, logits);

            float probs[NUM_ACTIONS];
            softmax(logits, probs);

            float val;
            compute_value(s_vec, w_value, val);

            // --------------------------------------------------------
            // c) deterministic action selection (argmax)
            // --------------------------------------------------------
            int chosen_action = 0;
            for (int a = 1; a < NUM_ACTIONS; ++a)
                if (probs[a] > probs[chosen_action])
                    chosen_action = a;

            // --------------------------------------------------------
            // d) store trajectory data
            // --------------------------------------------------------
            state_seq[step_cnt] = cur_state;
            action_seq[step_cnt] = chosen_action;
            reward_seq[step_cnt] = 0.0f;                 // placeholder
            value_seq[step_cnt] = val;
            for (int a = 0; a < NUM_ACTIONS; ++a)
                prob_seq[step_cnt][a] = probs[a];

            // --------------------------------------------------------
            // e) environment step
            // --------------------------------------------------------
            int nxt_state;
            float rew;
            step_environment(cur_state, chosen_action, nxt_state, rew);

            // store actual reward
            reward_seq[step_cnt] = rew;

            // move to next state
            cur_state = nxt_state;
            ++step_cnt;
        }

        // ----------------------------------------------------------------
        // 4. Back‑propagate advantages and update parameters
        // ----------------------------------------------------------------
        int back_idx = STEPS_PER_EP - 1;
        while (back_idx >= 0)
        {
            // a) current state and value
            int s_idx = state_seq[back_idx];
            float s_vec[NUM_STATES];
            make_state_vector(s_idx, s_vec);
            float v_curr = value_seq[back_idx];

            // b) next value (0 if last step)
            float v_next = 0.0f;
            if (back_idx < STEPS_PER_EP - 1)
                v_next = value_seq[back_idx + 1];

            // c) advantage = r + γ * V(next) - V(current)
            float adv = reward_seq[back_idx] + GAMMA * v_next - v_curr;

            // d) policy update
            int a_taken = action_seq[back_idx];
            float probs[NUM_ACTIONS];
            for (int a = 0; a < NUM_ACTIONS; ++a)
                probs[a] = prob_seq[back_idx][a];
            update_policy(w_policy, s_vec, probs, a_taken, adv, LR_POLICY);

            // e) value update
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
        // build state vector
        float s_vec[NUM_STATES];
        make_state_vector(s, s_vec);

        // policy
        float logits[NUM_ACTIONS];
        compute_logits(s_vec, w_policy, logits);
        float probs[NUM_ACTIONS];
        softmax(logits, probs);

        // value
        float val;
        compute_value(s_vec, w_value, val);

        std::cout << "State " << s << ":\n";
        std::cout << "  Policy ->";
        for (int a = 0; a < NUM_ACTIONS; ++a)
            std::cout << " Action" << a << ": " << probs[a] << (a + 1 == NUM_ACTIONS ? "\n" : ",");
        std::cout << "  Value  -> " << val << "\n";
    }

    return 0;
}
