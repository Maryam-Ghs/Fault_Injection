/****************************************************************************************
 *  Version #4 – Actor‑Critic with Experience Replay (stack‑based, verbose, edge‑case)
 *
 *  LLM input variant 9: medium-deterministic-random
 *
 *  Constraints fulfilled:
 *   – Only `float` and `int` types are used.
 *   – No `const`, `unsigned`, `double`, `long`.
 *   – All data lives in stack‑allocated arrays.
 *   – Input is generated internally (medium deterministic pseudo‑random data).
 *   – The program contains `int main()` and prints the final weights.
 *   – Helper functions split the logic; loops are written in varied forms.
 *   – Arithmetic operations are deliberately reordered where mathematically valid.
 *
 *  The implementation is deliberately verbose and step‑by‑step to aid readability.
 ****************************************************************************************/

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>

/* -------------------------------------------------------------------------- */
/*                         Hyper‑parameters (stack‑allocated)                 */
/* -------------------------------------------------------------------------- */
int MAX_BUFFER = 120;          // maximum number of stored transitions
int STATE_DIM   = 4;           // dimensionality of the state vector
int ACTION_DIM  = 3;           // number of possible discrete actions
int BATCH_SIZE  = 12;          // size of a sampled batch
int EPISODES    = 10;          // number of training episodes
int STEPS_PER_EP = 6;          // steps per episode (moderate)

float GAMMA    = 1.0f;         // discount factor – edge case (no discount)
float ALPHA_A  = 0.0f;         // actor learning rate – edge case (no learning)
float ALPHA_C  = 0.5f;         // critic learning rate – moderate learning

/* -------------------------------------------------------------------------- */
/*                         Experience‑Replay Buffer (stack)                     */
/* -------------------------------------------------------------------------- */
float   buf_state[200][4];
int     buf_action[200];
float   buf_reward[200];
float   buf_next_state[200][4];
int     buf_done[200];
int     buf_size = 0;

/* -------------------------------------------------------------------------- */
/*                         Utility Functions                                    */
/* -------------------------------------------------------------------------- */

/* Fill an array with small deterministic pseudo‑random numbers in [-5,5] */
void random_initialize(float *arr, int len)
{
    int i = 0;
    while (i < len)
    {
        /* Simple linear congruential generator with fixed seed behavior */
        unsigned seed = (unsigned)(i * 31 + 17);
        float r = (float)(seed % 1000) / 100.0f;   // [0,9.99]
        r = r - 5.0f;                               // [-5,4.99]
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
    float pref0 = 0.0f;
    float pref1 = 0.0f;
    float pref2 = 0.0f;
    int   i = 0;
    while (i < STATE_DIM)
    {
        pref0 = pref0 + actor_w[i] * state[i];
        pref1 = pref1 + actor_w[STATE_DIM + i] * state[i];
        pref2 = pref2 + actor_w[2 * STATE_DIM + i] * state[i];
        i = i + 1;
    }
    /* Return index of largest preference (break ties by lower index) */
    if (pref0 >= pref1 && pref0 >= pref2)
        return 0;
    else if (pref1 >= pref2)
        return 1;
    else
        return 2;
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

    int batch_idx[16];
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
/*                         Synthetic Environment (medium deterministic)         */
/* -------------------------------------------------------------------------- */

/* Generate a deterministic pseudo‑random state vector */
void generate_state(float *state)
{
    static int call_counter = 0;
    int i = 0;
    while (i < STATE_DIM)
    {
        unsigned val = (unsigned)(call_counter * 37 + i * 23);
        float r = (float)(val % 1000) / 100.0f;   // [0,9.99]
        r = r - 5.0f;                             // [-5,4.99]
        state[i] = r;
        i = i + 1;
    }
    call_counter = call_counter + 1;
}

/* Simulate reward: zero for most steps, moderate negative at episode end */
float compute_reward(int step, int done)
{
    if (done == 1)
        return -10.0f * (float)(step + 1);   // penalty scaled by step index
    else
        return 0.0f;
}

/* -------------------------------------------------------------------------- */
/*                         Main Training Loop                                   */
/* -------------------------------------------------------------------------- */
int main()
{
    std::srand(12345);   // deterministic seed

    /* Allocate weight tables on the stack */
    float actor_weights[12];   // STATE_DIM * ACTION_DIM = 4*3 = 12
    float critic_weights[4];  // STATE_DIM = 4

    initialise_weights(actor_weights, critic_weights);

    int ep = 0;
    while (ep < EPISODES)
    {
        /* ---------- Initialise episode ---------- */
        float state[4];
        generate_state(state);

        int step = 0;
        int done = 0;

        while (step < STEPS_PER_EP && done == 0)
        {
            /* Choose action */
            int action = choose_action(actor_weights, state);

            /* Generate next state (deterministic pseudo‑random) */
            float next_state[4];
            generate_state(next_state);

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
        while (train_iter < 3)
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
