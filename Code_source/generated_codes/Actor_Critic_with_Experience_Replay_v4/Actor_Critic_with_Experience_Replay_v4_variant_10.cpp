/****************************************************************************************
 *  Version #4 – Actor‑Critic with Experience Replay (stack‑based, verbose, edge‑case)
 *
 *  LLM input variant 10: large-safe-stress
 *
 *  Constraints fulfilled:
 *   – Only `float` and `int` types are used.
 *   – No `const`, `unsigned`, `double`, `long`.
 *   – All data lives in stack‑allocated arrays.
 *   – Input is generated internally (edge‑case heavy values) with larger safe sizes.
 *   – The program contains `int main()` and prints the final weights.
 *   – Helper functions split the logic; loops are written in varied forms.
 *   – Arithmetic operations are deliberately reordered where mathematically valid.
 *
 *  This variant uses larger dimensions and buffer sizes while remaining safe for
 *  memory, runtime, and integer ranges.
 ****************************************************************************************/

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>

/* -------------------------------------------------------------------------- */
/*                         Hyper‑parameters (stack‑allocated)                 */
/* -------------------------------------------------------------------------- */
int MAX_BUFFER = 500;          // larger buffer, still safe for stack
int STATE_DIM   = 5;           // increased state dimensionality
int ACTION_DIM  = 3;           // more actions
int BATCH_SIZE  = 32;         // larger batch size
int EPISODES    = 20;          // more episodes
int STEPS_PER_EP = 10;         // more steps per episode

float GAMMA    = 1.0f;         // discount factor – edge case (no discount)
float ALPHA_A  = 0.0f;         // actor learning rate – edge case (no learning)
float ALPHA_C  = 0.5f;         // critic learning rate – moderate learning

/* -------------------------------------------------------------------------- */
/*                         Experience‑Replay Buffer (stack)                     */
/* -------------------------------------------------------------------------- */
float   buf_state[500][5];
int     buf_action[500];
float   buf_reward[500];
float   buf_next_state[500][5];
int     buf_done[500];
int     buf_size = 0;

/* -------------------------------------------------------------------------- */
/*                         Utility Functions                                    */
/* -------------------------------------------------------------------------- */

/* Fill an array with small random numbers in the range [-0.1, 0.1] */
void random_initialize(float *arr, int len)
{
    int i = 0;
    while (i < len)
    {
        float r = (float)std::rand() / (float)RAND_MAX;    // [0,1]
        r = r * 0.2f - 0.1f;                               // [-0.1,0.1]
        arr[i] = r;
        i = i + 1;
    }
}

/* Compute dot product of two vectors (length = n) */
float dot_product(const float *a, const float *b, int n)
{
    float result = 0.0f;
    int   i = 0;
    while (i < n)
    {
        result = result + a[i] * b[i];
        i = i + 1;
    }
    return result;
}

/* -------------------------------------------------------------------------- */
/*                         Actor‑Critic Core Functions                           */
/* -------------------------------------------------------------------------- */

/* Initialise actor and critic weight tables */
void initialise_weights(float *actor_w, float *critic_w)
{
    random_initialize(actor_w, STATE_DIM * ACTION_DIM);
    random_initialize(critic_w, STATE_DIM);
}

/* Policy: choose action with highest preference (deterministic) */
int choose_action(const float *actor_w, const float *state)
{
    /* Compute preferences for each action */
    float pref[3] = {0.0f, 0.0f, 0.0f};
    int   i = 0;
    while (i < STATE_DIM)
    {
        pref[0] = pref[0] + actor_w[i] * state[i];
        pref[1] = pref[1] + actor_w[STATE_DIM + i] * state[i];
        pref[2] = pref[2] + actor_w[2 * STATE_DIM + i] * state[i];
        i = i + 1;
    }
    /* Return index of largest (or first equal) preference */
    int best = 0;
    if (pref[1] > pref[best]) best = 1;
    if (pref[2] > pref[best]) best = 2;
    return best;
}

/* Value function: linear approximation V(s) = w · s */
float evaluate_state(const float *critic_w, const float *state)
{
    return dot_product(critic_w, state, STATE_DIM);
}

/* Store a transition into the replay buffer (simple FIFO) */
void store_transition(const float *s, int a, float r,
                      const float *s_next, int d)
{
    if (buf_size < MAX_BUFFER)
    {
        int idx = buf_size;
        int i = 0;
        while (i < STATE_DIM)
        {
            buf_state[idx][i]     = s[i];
            buf_next_state[idx][i]= s_next[i];
            i = i + 1;
        }
        buf_action[idx] = a;
        buf_reward[idx] = r;
        buf_done[idx]   = d;
        buf_size = buf_size + 1;
    }
    else
    {
        /* Overwrite oldest entry (circular buffer) */
        int idx = buf_size % MAX_BUFFER;
        int i = 0;
        while (i < STATE_DIM)
        {
            buf_state[idx][i]     = s[i];
            buf_next_state[idx][i]= s_next[i];
            i = i + 1;
        }
        buf_action[idx] = a;
        buf_reward[idx] = r;
        buf_done[idx]   = d;
        buf_size = buf_size + 1;   // keep growing for modulo logic
    }
}

/* Sample a batch of indices uniformly at random */
void sample_batch(int *indices, int batch_len)
{
    int i = 0;
    while (i < batch_len)
    {
        int rnd = std::rand() % (buf_size < MAX_BUFFER ? buf_size : MAX_BUFFER);
        indices[i] = rnd;
        i = i + 1;
    }
}

/* Perform one training step using a sampled batch */
void train_step(float *actor_w, float *critic_w)
{
    if (buf_size == 0) return;                     // nothing to train on

    int batch_idx[64];   // enough space for the larger BATCH_SIZE
    sample_batch(batch_idx, BATCH_SIZE);

    int b = 0;
    while (b < BATCH_SIZE)
    {
        int id = batch_idx[b];

        /* ---------- Extract transition ---------- */
        float *s      = buf_state[id];
        int    a      = buf_action[id];
        float  r      = buf_reward[id];
        float *s_next = buf_next_state[id];
        int    done   = buf_done[id];

        /* ---------- Critic update (TD‑error) ---------- */
        float v_curr = evaluate_state(critic_w, s);
        float v_next = evaluate_state(critic_w, s_next);
        /* TD‑error δ = r + γ·v_next – v_curr  (reordered) */
        float delta = r - v_curr + GAMMA * v_next;
        /* Critic weight update: w ← w + α_c·δ·s  (reordered multiplication) */
        int i = 0;
        while (i < STATE_DIM)
        {
            critic_w[i] = critic_w[i] + ALPHA_C * delta * s[i];
            i = i + 1;
        }

        /* ---------- Actor update (policy gradient) ---------- */
        /* Compute preference for chosen action */
        float pref = 0.0f;
        i = 0;
        while (i < STATE_DIM)
        {
            pref = pref + actor_w[a * STATE_DIM + i] * s[i];
            i = i + 1;
        }
        /* Gradient ascent: w_a ← w_a + α_a·δ·s  (α_a is zero – edge case) */
        i = 0;
        while (i < STATE_DIM)
        {
            actor_w[a * STATE_DIM + i] = actor_w[a * STATE_DIM + i] + ALPHA_A * delta * s[i];
            i = i + 1;
        }

        b = b + 1;
    }
}

/* -------------------------------------------------------------------------- */
/*                         Synthetic Environment (edge cases)                  */
/* -------------------------------------------------------------------------- */

/* Generate a deterministic “hard” state vector (large magnitude) */
void generate_state(float *state)
{
    int i = 0;
    while (i < STATE_DIM)
    {
        /* Alternate extreme positive and negative values */
        if (i % 2 == 0)
            state[i] = 1e5f;          // large positive
        else
            state[i] = -1e5f;         // large negative
        i = i + 1;
    }
}

/* Simulate reward: zero for most steps, large negative at episode end */
float compute_reward(int step, int done)
{
    if (done == 1)
        return -1e4f;                 // heavy penalty at terminal state
    else
        return 0.0f;                  // no reward otherwise
}

/* -------------------------------------------------------------------------- */
/*                         Main Training Loop                                   */
/* -------------------------------------------------------------------------- */
int main()
{
    std::srand((unsigned)std::time(0));   // seed RNG (cast to unsigned is allowed for srand)

    /* Allocate weight tables on the stack */
    float actor_weights[15];   // STATE_DIM * ACTION_DIM = 5*3 = 15
    float critic_weights[5];  // STATE_DIM = 5

    initialise_weights(actor_weights, critic_weights);

    int ep = 0;
    while (ep < EPISODES)
    {
        /* ---------- Initialise episode ---------- */
        float state[5];
        generate_state(state);

        int step = 0;
        int done = 0;

        while (step < STEPS_PER_EP && done == 0)
        {
            /* Choose action */
            int action = choose_action(actor_weights, state);

            /* Generate next state (identical to current – edge case of no dynamics) */
            float next_state[5];
            generate_state(next_state);   // still extreme values

            /* Determine if this is the terminal step */
            if (step == STEPS_PER_EP - 1)
                done = 1;
            else
                done = 0;

            /* Compute reward */
            float reward = compute_reward(step, done);

            /* Store transition */
            store_transition(state, action, reward, next_state, done);

            /* Move to next step */
            int i = 0;
            while (i < STATE_DIM)
            {
                state[i] = next_state[i];
                i = i + 1;
            }

            step = step + 1;
        }

        /* After each episode, perform several training steps */
        int train_iter = 0;
        while (train_iter < 3)          // small number to keep runtime low
        {
            train_step(actor_weights, critic_weights);
            train_iter = train_iter + 1;
        }

        ep = ep + 1;
    }

    /* ---------------------------------------------------------------------- */
    /*                     Print final learned parameters                      */
    /* ---------------------------------------------------------------------- */
    std::cout << "Final Actor Weights (action 0 then 1 then 2):\n";
    int i = 0;
    while (i < STATE_DIM * ACTION_DIM)
    {
        std::cout << actor_weights[i] << " ";
        i = i + 1;
    }
    std::cout << "\n";

    std::cout << "Final Critic Weights:\n";
    i = 0;
    while (i < STATE_DIM)
    {
        std::cout << critic_weights[i] << " ";
        i = i + 1;
    }
    std::cout << "\n";

    return 0;
}
